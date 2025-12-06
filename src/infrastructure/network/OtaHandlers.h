#pragma once
#include <Arduino.h>

#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <Update.h>
#include "./common/Constants.h"
#include "./ApiHelpers.h"

class OtaHandlers
{
private:
    AsyncWebServer &server;

    struct OtaState
    {
        bool inProgress = false;
        uint32_t startTime = 0;
        size_t totalSize = 0;
        size_t receivedSize = 0;
        int lastProgress = -1;
        bool rebootScheduled = false;
        uint32_t rebootTime = 0;
    } otaState;

public:
    OtaHandlers(AsyncWebServer &serv) : server(serv) {}

    void setupEndpoints()
    {
        server.on("/ota", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
      if (LittleFS.exists("/ota.html")) {
        request -> send(LittleFS, "/ota.html", "text/html");
      } else {
        request -> send(404, "text/plain", "OTA page not found");
      } });

        // OTA обновление
        server.on("/api/system/update", HTTP_POST, [this](AsyncWebServerRequest *request)
                  { handleOtaRequest(request); }, [this](AsyncWebServerRequest *request, const String &filename, size_t index, uint8_t *data, size_t len, bool final)
                  { handleOtaUpload(request, filename, index, data, len, final); });

        server.on("/api/system/update/status", HTTP_GET,
                  [this](AsyncWebServerRequest *request)
                  {
                      handleOtaStatus(request);
                  });

        server.on("/api/system/update/cancel", HTTP_POST,
                  [this](AsyncWebServerRequest *request)
                  {
                      if (otaState.inProgress)
                      {
                          Update.end(false);
                          otaState.inProgress = false;
                          ApiHelpers::sendJsonResponse(request, "{\"status\":\"cancelled\"}");
                      }
                  });
    }

    void process()
    {
        // Проверяем, нужно ли выполнить отложенную перезагрузку
        if (otaState.rebootScheduled && millis() >= otaState.rebootTime)
        {
            Serial.println("🔄 Executing scheduled reboot...");
            ESP.restart();
        }
    }

private:
    // Проверка авторизации
    bool checkAuth(AsyncWebServerRequest *request)
    {
        if (!request->authenticate(OTA_USERNAME, OTA_PASSWORD))
        {
            request->requestAuthentication();
            return false;
        }
        return true;
    }

    // Обработка OTA запроса (без файла)
    void handleOtaRequest(AsyncWebServerRequest *request)
    {
        if (!checkAuth(request))
            return;

        if (otaState.inProgress)
        {
            ApiHelpers::sendJsonError(request, "OTA update already in progress", 400);
            return;
        }
    }

    // Обработка загрузки файла OTA
    void handleOtaUpload(AsyncWebServerRequest *request,
                         const String &filename,
                         size_t index, uint8_t *data, size_t len, bool final)
    {
        if (!checkAuth(request))
            return;

        // Начало загрузки
        if (index == 0)
        {
            if (!beginOtaUpdate(request, filename))
                return;
        }

        // Если OTA не начата, игнорируем
        if (!otaState.inProgress)
            return;

        // Запись данных
        if (len > 0 && !writeOtaData(data, len, request))
            return;

        // Завершение
        if (final)
        {
            finalizeOtaUpdate(request, filename);
        }
    }

    // Начало OTA обновления
    bool beginOtaUpdate(AsyncWebServerRequest *request,
                        const String &filename)
    {
        // Проверка расширения файла
        if (!filename.endsWith(".bin"))
        {
            ApiHelpers::sendJsonError(request, "Invalid file type. Only .bin files allowed", 400);
            return false;
        }

        otaState.totalSize = request->contentLength();
        otaState.receivedSize = 0;
        otaState.lastProgress = -1;
        otaState.rebootScheduled = false;

        // Проверка размера
        if (otaState.totalSize == 0)
        {
            ApiHelpers::sendJsonError(request, "Empty file", 400);
            return false;
        }

        size_t maxSize = ESP.getFreeSketchSpace() - 0x1000;
        if (otaState.totalSize > maxSize)
        {
            DynamicJsonDocument doc(256);
            doc["error"] = "File too large";
            doc["maxSize"] = maxSize;
            doc["receivedSize"] = otaState.totalSize;
            ApiHelpers::sendJsonDocument(request, doc, 400);
            return false;
        }

        Serial.printf("📦 OTA Started: %s (%u bytes)\n", filename.c_str(), otaState.totalSize);

        // Инициализация обновления
        Update.onProgress([this](size_t progress, size_t total)
                          { this->handleOtaProgress(progress, total); });

        if (!Update.begin(otaState.totalSize, U_FLASH))
        {
            ApiHelpers::sendJsonError(request,
                                      "Update begin failed: " + String(Update.errorString()), 400);
            return false;
        }

        otaState.inProgress = true;
        otaState.startTime = millis();
        Serial.println("✅ OTA update initialized");
        return true;
    }

    // Обработка прогресса OTA
    void handleOtaProgress(size_t progress, size_t total)
    {
        int percent = (progress * 100) / total;
        if (percent != otaState.lastProgress)
        {
            otaState.lastProgress = percent;
            if (percent % 10 == 0 || percent == 100) // Логируем каждые 10%
            {
                Serial.printf("📥 OTA Progress: %d%% (%u/%u bytes)\n",
                              percent, progress, total);
            }
        }
    }

    // Запись данных OTA
    bool writeOtaData(uint8_t *data, size_t len, AsyncWebServerRequest *request)
    {
        size_t written = Update.write(data, len);
        if (written != len)
        {
            DynamicJsonDocument doc(256);
            doc["error"] = "Write failed";
            doc["written"] = written;
            doc["expected"] = len;
            doc["updateError"] = String(Update.errorString());

            ApiHelpers::sendJsonDocument(request, doc, 400);

            Update.end(false);
            otaState.inProgress = false;
            return false;
        }

        otaState.receivedSize += len;
        return true;
    }

    // Завершение OTA обновления
    void finalizeOtaUpdate(AsyncWebServerRequest *request,
                           const String &filename)
    {
        Serial.println("✅ Firmware upload complete, finalizing...");

        if (Update.end(true))
        {
            sendOtaSuccess(request, filename);
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
            int progress = (otaState.totalSize > 0) ? (otaState.receivedSize * 100) / otaState.totalSize : 0;

            doc["status"] = "in_progress";
            doc["progress"] = progress;
            doc["receivedBytes"] = otaState.receivedSize;
            doc["totalBytes"] = otaState.totalSize;
            doc["elapsedTimeSeconds"] = (millis() - otaState.startTime) / 1000;

            if (otaState.receivedSize > 0 && (millis() - otaState.startTime) > 0)
            {
                uint32_t elapsedTime = millis() - otaState.startTime;
                uint32_t bytesPerSecond = (otaState.receivedSize * 1000) / elapsedTime;
                uint32_t remainingBytes = otaState.totalSize - otaState.receivedSize;

                if (bytesPerSecond > 0)
                {
                    doc["speedBytesPerSecond"] = bytesPerSecond;
                    doc["speedFormatted"] = formatBytes(bytesPerSecond) + "/s";
                    doc["estimatedTimeRemainingSeconds"] = remainingBytes / bytesPerSecond;
                }
            }
        }
        else
        {
            doc["status"] = "idle";
            doc["firmwareVersion"] = FIRMWARE_VERSION;
            doc["freeSketchSpace"] = ESP.getFreeSketchSpace();
            doc["freeSketchSpaceFormatted"] = formatBytes(ESP.getFreeSketchSpace());
            doc["maxOtaSize"] = ESP.getFreeSketchSpace() - 0x1000;
            doc["maxOtaSizeFormatted"] = formatBytes(ESP.getFreeSketchSpace() - 0x1000);
            doc["rebootScheduled"] = otaState.rebootScheduled;

            if (otaState.rebootScheduled)
            {
                doc["rebootInSeconds"] = (otaState.rebootTime - millis()) / 1000;
            }
        }

        ApiHelpers::sendJsonDocument(request, doc);
    }

    // Отправка успешного ответа OTA (без немедленной перезагрузки)
    void sendOtaSuccess(AsyncWebServerRequest *request,
                        const String &filename)
    {
        DynamicJsonDocument doc(512);
        doc["status"] = "success";
        doc["message"] = "Firmware updated successfully. System will reboot in 3 seconds.";
        doc["filename"] = filename;
        doc["bytesReceived"] = otaState.receivedSize;
        doc["bytesReceivedFormatted"] = formatBytes(otaState.receivedSize);
        doc["durationMs"] = millis() - otaState.startTime;
        doc["durationSeconds"] = (millis() - otaState.startTime) / 1000;
        doc["rebootTime"] = millis() + 3000; // Перезагрузка через 3 секунды

        // Запланировать отложенную перезагрузку
        otaState.rebootScheduled = true;
        otaState.rebootTime = millis() + 3000; // 3 секунды на отправку ответа

        Serial.printf("🎉 OTA Success: %s (%u bytes in %u ms)\n",
                      filename.c_str(), otaState.receivedSize, millis() - otaState.startTime);
        Serial.println("🔄 Scheduled reboot in 3 seconds...");

        // Отправляем ответ с заголовком Connection: close
        AsyncWebServerResponse *response = request->beginResponse(200, "application/json",
                                                                  jsonDocumentToString(doc));
        response->addHeader("Connection", "close");
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    }

    // Вспомогательный метод для форматирования байтов
    String formatBytes(size_t bytes)
    {
        if (bytes < 1024)
            return String(bytes) + " B";
        else if (bytes < 1024 * 1024)
            return String(bytes / 1024.0, 1) + " KB";
        else if (bytes < 1024 * 1024 * 1024)
            return String(bytes / (1024.0 * 1024.0), 1) + " MB";
        else
            return String(bytes / (1024.0 * 1024.0 * 1024.0), 1) + " GB";
    }

    String jsonDocumentToString(DynamicJsonDocument &doc)
    {
        String json;
        serializeJson(doc, json);
        return json;
    }
};