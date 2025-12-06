#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <Update.h>
#include "./common/Version.h"
#include "./common/Utils.h"
#include "./ApiHelpers.h"

class OtaHandlers
{
private:
    AsyncWebServer &server;

    struct OtaState
    {
        bool inProgress = false;
        size_t totalSize = 0;
        size_t receivedSize = 0;
        uint32_t startTime = 0;
        bool rebootScheduled = false;
        uint32_t rebootTime = 0;
    } otaState;

public:
    OtaHandlers(AsyncWebServer &serv) : server(serv) {}

    void setupEndpoints()
    {
        // Главная OTA страница
        server.on("/ota", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
            if (LittleFS.exists("/ota.html")) {
                request->send(LittleFS, "/ota.html", "text/html");
            } else {
                request->send(404, "text/plain", "OTA page not found");
            } });

        // Запрос на обновление
        server.on("/api/system/update", HTTP_POST, [this](AsyncWebServerRequest *request)
                  {
                // Проверяем, не идет ли уже обновление
                if (otaState.inProgress) {
                    ApiHelpers::sendJsonError(request, "OTA update already in progress", 400);
                    return;
                } },
                  // Обработчик загрузки файла
                  [this](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
                  { handleOtaUpload(request, filename, index, data, len, final); });

        // Статус OTA
        server.on("/api/system/update/status", HTTP_GET,
                  [this](AsyncWebServerRequest *request)
                  {
                      handleOtaStatus(request);
                  });
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

private:
    // Основной обработчик загрузки OTA
    void handleOtaUpload(AsyncWebServerRequest *request, const String &filename,
                         size_t index, uint8_t *data, size_t len, bool final)
    {
        // Начало загрузки
        if (index == 0)
        {
            if (!beginOtaUpdate(request, filename))
            {
                return;
            }
        }

        // Пропускаем если OTA не начата
        if (!otaState.inProgress)
            return;

        // Записываем данные
        if (len > 0 && !writeOtaData(data, len))
        {
            otaState.inProgress = false;
            return;
        }

        // Завершение загрузки
        if (final)
        {
            finalizeOtaUpdate(request, filename);
        }
    }

    // Инициализация OTA обновления
    bool beginOtaUpdate(AsyncWebServerRequest *request, const String &filename)
    {
        // Проверка типа файла
        if (!filename.endsWith(".bin"))
        {
            ApiHelpers::sendJsonError(request, "Invalid file type. Only .bin files allowed", 400);
            return false;
        }

        // Получаем размер файла
        otaState.totalSize = request->contentLength();
        if (otaState.totalSize == 0)
        {
            ApiHelpers::sendJsonError(request, "Empty file", 400);
            return false;
        }

        // Проверяем размер
        size_t maxSize = ESP.getFreeSketchSpace() - 0x1000;
        if (otaState.totalSize > maxSize)
        {
            ApiHelpers::sendJsonError(request,
                                      "File too large. Max: " + String(maxSize) + " bytes, Got: " + String(otaState.totalSize) + " bytes",
                                      400);
            return false;
        }

        // Начинаем обновление
        if (!Update.begin(otaState.totalSize, U_FLASH))
        {
            ApiHelpers::sendJsonError(request,
                                      "Update begin failed: " + String(Update.errorString()), 400);
            return false;
        }

        // Настраиваем коллбэк прогресса
        Update.onProgress([this](size_t progress, size_t total)
                          {
            if ((progress * 100 / total) % 10 == 0) {
                Serial.printf("📥 OTA Progress: %d%% (%u/%u bytes)\n", 
                    (progress * 100) / total, progress, total);
            } });

        // Инициализируем состояние
        otaState.inProgress = true;
        otaState.receivedSize = 0;
        otaState.startTime = millis();
        otaState.rebootScheduled = false;

        Serial.printf("📦 OTA Started: %s (%u bytes)\n", filename.c_str(), otaState.totalSize);
        return true;
    }

    // Запись данных
    bool writeOtaData(uint8_t *data, size_t len)
    {
        size_t written = Update.write(data, len);
        if (written != len)
        {
            Serial.printf("❌ Write failed: written %u, expected %u\n", written, len);
            Update.end(false);
            return false;
        }

        otaState.receivedSize += len;
        return true;
    }

    // Завершение обновления
    void finalizeOtaUpdate(AsyncWebServerRequest *request, const String &filename)
    {
        Serial.println("✅ Firmware upload complete, finalizing...");

        if (Update.end(true))
        {
            sendSuccessResponse(request, filename);
        }
        else
        {
            ApiHelpers::sendJsonError(request,
                                      "Update finalization failed: " + String(Update.errorString()), 400);
            Serial.println("❌ OTA update failed: " + String(Update.errorString()));
        }

        otaState.inProgress = false;
    }

    // Статус OTA
    void handleOtaStatus(AsyncWebServerRequest *request)
    {
        DynamicJsonDocument doc(512);

        if (otaState.inProgress)
        {
            int progress = otaState.totalSize > 0 ? (otaState.receivedSize * 100) / otaState.totalSize : 0;

            doc["status"] = "in_progress";
            doc["progress"] = progress;
            doc["receivedBytes"] = otaState.receivedSize;
            doc["totalBytes"] = otaState.totalSize;
            doc["elapsedTime"] = (millis() - otaState.startTime) / 1000;

            // Скорость загрузки
            if (otaState.receivedSize > 0)
            {
                uint32_t elapsed = millis() - otaState.startTime;
                uint32_t speed = elapsed > 0 ? (otaState.receivedSize * 1000) / elapsed : 0;
                doc["speedBps"] = speed;
                doc["speed"] = Utils::formatSizeBytes(speed) + "/s";
            }
        }
        else
        {
            doc["status"] = "idle";
            doc["firmwareVersion"] = FIRMWARE_VERSION;
            doc["freeSpace"] = ESP.getFreeSketchSpace();
            doc["maxOtaSize"] = ESP.getFreeSketchSpace() - 0x1000;
            doc["rebootScheduled"] = otaState.rebootScheduled;

            if (otaState.rebootScheduled)
            {
                doc["rebootIn"] = (otaState.rebootTime - millis()) / 1000;
            }
        }

        ApiHelpers::sendJsonDocument(request, doc);
    }

    // Успешный ответ
    void sendSuccessResponse(AsyncWebServerRequest *request, const String &filename)
    {
        DynamicJsonDocument doc(256);
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

        // Отправляем ответ и закрываем соединение
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json");
        doc.shrinkToFit();
        serializeJson(doc, response);
        response->addHeader("Connection", "close");
        request->send(response);
    }
};