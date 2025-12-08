#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <Update.h>
#include "./WebSocketManager.h"
#include "./common/Version.h"
#include "./common/Utils.h"
#include "./ApiHelpers.h"

class OtaHandlers
{
private:
    AsyncWebServer &server;
    WebSocketManager &webSocketManager;
    ConfigManager &configManager;
    FileSystemManager &fsManager;

    struct OtaState
    {
        bool inProgress = false;
        size_t totalSize = 0;
        size_t receivedSize = 0;
        uint32_t startTime = 0;
        bool rebootScheduled = false;
        uint32_t rebootTime = 0;
        int lastBroadcastProgress = -1; // Для предотвращения спама
    } otaState;

public:
    OtaHandlers(AsyncWebServer &serv, WebSocketManager &wsMngr, ConfigManager &configMngr, FileSystemManager &fsMgr)
        : server(serv), webSocketManager(wsMngr), configManager(configMngr), fsManager(fsMgr) {}

    void setupEndpoints()
    {
        // OTA обновление
        server.on("/api/system/update", HTTP_POST, [this](AsyncWebServerRequest *request)
                  {
                    //   if (!request->authenticate(
                    //       configManager.getConfig().network.otaUsername.c_str(),
                    //       configManager.getConfig().network.otaPassword.c_str()))
                    //   {
                    //       return request->requestAuthentication();
                    //   }

                if (otaState.inProgress) {
                    ApiHelpers::sendJsonError(request, "OTA update already in progress", 400);
                    return;
                } }, [this](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
                  { handleOtaUpload(request, filename, index, data, len, final); });
    }

    void process()
    {
        // Проверяем отложенную перезагрузку
        if (otaState.rebootScheduled && millis() >= otaState.rebootTime)
        {
            Serial.println("🔄 Executing scheduled reboot...");
            ESP.restart();
        }
    }

    // Метод для отправки прогресса через WebSocket
    void broadcastProgress()
    {
        if (!otaState.inProgress || otaState.totalSize == 0)
            return;

        int progress = (otaState.receivedSize * 100) / otaState.totalSize;

        // Отправляем только если прогресс изменился на 1% или больше
        if (abs(progress - otaState.lastBroadcastProgress) >= 1)
        {
            otaState.lastBroadcastProgress = progress;

            DynamicJsonDocument doc(256);

            doc["progress"] = progress;
            doc["received"] = otaState.receivedSize;
            doc["total"] = otaState.totalSize;

            // Рассчитываем скорость
            if (otaState.receivedSize > 0)
            {
                uint32_t elapsed = millis() - otaState.startTime;
                if (elapsed > 0)
                {
                    uint32_t speed = (otaState.receivedSize * 1000) / elapsed;
                    doc["speed"] = speed;

                    // Оставшееся время
                    if (speed > 0)
                    {
                        uint32_t remaining = (otaState.totalSize - otaState.receivedSize) / speed;
                        doc["remaining"] = remaining;
                    }
                }
            }

            // Также выводим в Serial для отладки
            if (progress % 2 == 0)
            {
                String json;
                serializeJson(doc, json);
                webSocketManager.broadcastJson(EventType::OTA_PROGRESS, json);
                Serial.printf("📊 OTA Progress: %d%% (%u/%u bytes)\n",
                              progress, otaState.receivedSize, otaState.totalSize);
            }
        }
    }

private:
    // Обработчик загрузки
    void handleOtaUpload(AsyncWebServerRequest *request, const String &filename,
                         size_t index, uint8_t *data, size_t len, bool final)
    {
        if (index == 0)
        {
            if (!beginOtaUpdate(request, filename))
                return;
        }

        if (!otaState.inProgress)
            return;

        // Используем FileSystemManager для проверки свободного места
        if (final && len == 0)
        {
            // Проверка свободного места перед началом
            size_t totalBytes, usedBytes;
            fsManager.getInfo(totalBytes, usedBytes);
            size_t freeBytes = totalBytes - usedBytes;

            if (otaState.totalSize > freeBytes)
            {
                String error = "Not enough space. Need: " +
                               String(otaState.totalSize) +
                               " bytes, Free: " +
                               String(freeBytes) + " bytes";

                ApiHelpers::sendJsonError(request, error);
                otaState.inProgress = false;
                return;
            }
        }

        if (len > 0 && !writeOtaData(data, len))
        {
            otaState.inProgress = false;

            ApiHelpers::sendJsonError(request, "Write failed");
            return;
        }

        broadcastProgress();

        if (final)
        {
            finalizeOtaUpdate(request, filename);
        }
    }

    // Инициализация OTA
    bool beginOtaUpdate(AsyncWebServerRequest *request, const String &filename)
    {
        if (!filename.endsWith(".bin"))
        {
            ApiHelpers::sendJsonError(request, "Only .bin files allowed", 400);
            return false;
        }

        otaState.totalSize = request->contentLength();
        if (otaState.totalSize == 0)
        {
            ApiHelpers::sendJsonError(request, "Empty file", 400);
            return false;
        }

        size_t maxSize = ESP.getFreeSketchSpace() - 0x1000;
        if (otaState.totalSize > maxSize)
        {
            ApiHelpers::sendJsonError(request,
                                      String("File too large. Max: ") + String(maxSize) + " bytes", 400);
            return false;
        }

        if (!Update.begin(otaState.totalSize, U_FLASH))
        {
            ApiHelpers::sendJsonError(request,
                                      "Update begin failed: " + String(Update.errorString()), 400);
            return false;
        }

        otaState.inProgress = true;
        otaState.receivedSize = 0;
        otaState.startTime = millis();
        otaState.rebootScheduled = false;
        otaState.lastBroadcastProgress = -1;

        Serial.printf("📦 OTA Started: %s (%u bytes)\n", filename.c_str(), otaState.totalSize);
        return true;
    }

    // Запись данных
    bool writeOtaData(uint8_t *data, size_t len)
    {
        size_t written = Update.write(data, len);
        if (written != len)
        {
            Serial.printf("❌ Write failed: %u/%u bytes\n", written, len);
            Update.end(false);
            return false;
        }

        otaState.receivedSize += len;
        return true;
    }

    // Завершение
    void finalizeOtaUpdate(AsyncWebServerRequest *request, const String &filename)
    {
        Serial.println("✅ Finalizing update...");

        if (Update.end(true))
        {
            sendSuccessResponse(request, filename);
        }
        else
        {
            String error = "Update failed: " + String(Update.errorString());
            ApiHelpers::sendJsonError(request, error, 400);
        }

        otaState.inProgress = false;
    }

    // Успешный ответ
    void sendSuccessResponse(AsyncWebServerRequest *request, const String &filename)
    {
        DynamicJsonDocument doc(128);
        doc["status"] = "success";
        doc["message"] = "Firmware updated successfully. Rebooting...";
        doc["filename"] = filename;
        doc["size"] = otaState.receivedSize;
        doc["duration"] = (millis() - otaState.startTime) / 1000;

        // Планируем перезагрузку через 2 секунды
        otaState.rebootScheduled = true;
        otaState.rebootTime = millis() + 2000;

        Serial.printf("🎉 OTA Success: %s (%u bytes in %u ms)\n",
                      filename.c_str(), otaState.receivedSize, millis() - otaState.startTime);
        Serial.println("🔄 Scheduled reboot in 2 seconds...");

        String json;
        serializeJson(doc, json);

        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
        response->addHeader("Connection", "close");
        request->send(response);
    }
};