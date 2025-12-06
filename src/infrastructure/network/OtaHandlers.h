#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <Update.h>
#include "./common/Constants.h"

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
    } otaState;

public:
    OtaHandlers(AsyncWebServer &serv) : server(serv) {}

    void setupEndpoints()
    {

        server.on("/ota", HTTP_GET, [](AsyncWebServerRequest *request)
                  {
    if (LittleFS.exists("/ota.html")) {
        request->send(LittleFS, "/ota.html", "text/html");
    } else {
        request->send(404, "text/plain", "OTA page not found");
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
                          sendJsonResponse(request, "{\"status\":\"cancelled\"}");
                      }
                  });
    }

private:
    // Проверка авторизации (вынесено в отдельный метод)
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
            sendJsonError(request, "OTA update already in progress", 400);
            return;
        }

        sendJsonResponse(request,
                         "{\"status\":\"ready\",\"message\":\"Send firmware as multipart/form-data\","
                         "\"maxSize\":" +
                             String(ESP.getFreeSketchSpace() - 0x1000) + "}");
    }

    // Обработка загрузки файла OTA
    void handleOtaUpload(AsyncWebServerRequest *request, const String &filename,
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
        else
        {
            request->send(200); // Промежуточный OK
        }
    }

    // Начало OTA обновления
    bool beginOtaUpdate(AsyncWebServerRequest *request, const String &filename)
    {
        // Проверка расширения файла
        if (!filename.endsWith(".bin"))
        {
            sendJsonError(request, "Invalid file type. Only .bin files allowed", 400);
            return false;
        }

        otaState.totalSize = request->contentLength();
        otaState.receivedSize = 0;
        otaState.lastProgress = -1;

        // Проверка размера
        if (otaState.totalSize == 0)
        {
            sendJsonError(request, "Empty file", 400);
            return false;
        }

        size_t maxSize = ESP.getFreeSketchSpace() - 0x1000;
        if (otaState.totalSize > maxSize)
        {
            sendJsonError(request,
                          "File too large. Max: " + String(maxSize) +
                              ", received: " + String(otaState.totalSize),
                          400);
            return false;
        }

        Serial.printf("📦 OTA: %s (%u bytes)\n", filename.c_str(), otaState.totalSize);

        // Инициализация обновления
        if (!Update.begin(otaState.totalSize, U_FLASH))
        {
            sendJsonError(request, "Update begin failed: " + String(Update.errorString()), 400);
            return false;
        }

        otaState.inProgress = true;
        otaState.startTime = millis();
        Serial.println("✅ OTA update initialized");
        return true;
    }

    // Запись данных OTA
    bool writeOtaData(uint8_t *data, size_t len, AsyncWebServerRequest *request)
    {
        size_t written = Update.write(data, len);
        if (written != len)
        {
            sendJsonError(request,
                          "Write failed. Written: " + String(written) +
                              ", expected: " + String(len) +
                              ", error: " + String(Update.errorString()),
                          400);

            Update.end(false);
            otaState.inProgress = false;
            return false;
        }

        otaState.receivedSize += len;

        // Логирование прогресса
        logProgress();
        return true;
    }

    // Завершение OTA обновления
    void finalizeOtaUpdate(AsyncWebServerRequest *request, const String &filename)
    {
        Serial.println("✅ Firmware upload complete, finalizing...");

        if (Update.end(true))
        {
            sendOtaSuccess(request, filename);
            rebootAfterDelay(1000);
        }
        else
        {
            sendJsonError(request, "Update finalization failed: " + String(Update.errorString()), 400);
            Serial.println("❌ OTA update failed: " + String(Update.errorString()));
        }

        otaState.inProgress = false;
    }

    // Логирование прогресса OTA
    void logProgress()
    {
        int progress = (otaState.receivedSize * 100) / otaState.totalSize;

        if (progress != otaState.lastProgress && (progress % 5 == 0))
        {
            Serial.printf("📥 OTA: %d%% (%u/%u bytes)\n",
                          progress, otaState.receivedSize, otaState.totalSize);
            otaState.lastProgress = progress;
        }
    }

    // Статус OTA
    void handleOtaStatus(AsyncWebServerRequest *request)
    {
        DynamicJsonDocument doc(256);

        if (otaState.inProgress)
        {
            int progress = (otaState.totalSize > 0) ? (otaState.receivedSize * 100) / otaState.totalSize : 0;

            doc["status"] = "in_progress";
            doc["progress"] = progress;
            doc["receivedBytes"] = otaState.receivedSize;
            doc["totalBytes"] = otaState.totalSize;
            doc["elapsedTime"] = (millis() - otaState.startTime) / 1000;

            if (otaState.receivedSize > 0)
            {
                uint32_t remainingBytes = otaState.totalSize - otaState.receivedSize;
                uint32_t bytesPerSecond = otaState.receivedSize * 1000 /
                                          (millis() - otaState.startTime);

                if (bytesPerSecond > 0)
                {
                    doc["estimatedTimeRemaining"] = remainingBytes / bytesPerSecond;
                    doc["speedBytesPerSecond"] = bytesPerSecond;
                }
            }
        }
        else
        {
            doc["status"] = "idle";
            doc["firmwareVersion"] = FIRMWARE_VERSION;
            doc["freeSketchSpace"] = ESP.getFreeSketchSpace();
            doc["maxOtaSize"] = ESP.getFreeSketchSpace() - 0x1000;
        }

        sendJsonDocument(request, doc);
    }

    // Вспомогательные методы
    void sendOtaSuccess(AsyncWebServerRequest *request, const String &filename)
    {
        DynamicJsonDocument doc(256);
        doc["status"] = "success";
        doc["message"] = "Firmware updated successfully. Rebooting...";
        doc["bytesReceived"] = otaState.receivedSize;
        doc["filename"] = filename;
        doc["duration"] = (millis() - otaState.startTime) / 1000;

        Serial.printf("🎉 OTA Success: %u bytes in %u ms\n",
                      otaState.receivedSize, millis() - otaState.startTime);

        AsyncWebServerResponse *response = request->beginResponse(200, "application/json",
                                                                  jsonDocumentToString(doc));
        response->addHeader("Connection", "close");
        request->send(response);
    }

    void sendJsonError(AsyncWebServerRequest *request, const String &message, int code = 400)
    {
        DynamicJsonDocument doc(128);
        doc["error"] = message;
        sendJsonDocument(request, doc, code);
    }

    void sendJsonResponse(AsyncWebServerRequest *request, const String &json, int code = 200)
    {
        AsyncWebServerResponse *response = request->beginResponse(code, "application/json", json);
        response->addHeader("Access-Control-Allow-Origin", "*");
        request->send(response);
    }

    void sendJsonDocument(AsyncWebServerRequest *request, DynamicJsonDocument &doc, int code = 200)
    {
        String json;
        serializeJson(doc, json);
        sendJsonResponse(request, json, code);
    }

    String jsonDocumentToString(DynamicJsonDocument &doc)
    {
        String json;
        serializeJson(doc, json);
        return json;
    }

    void rebootAfterDelay(uint32_t delayMs)
    {
        // Отложенная перезагрузка в отдельной задаче
        delay(100); // Даем время на отправку ответа

        // Используем таймер для безопасной перезагрузки
        // (альтернатива: создать задачу FreeRTOS)
        Serial.printf("🔄 Rebooting in %u ms...\n", delayMs);

        // Простая реализация - в production лучше использовать FreeRTOS task
        uint32_t start = millis();
        while (millis() - start < delayMs)
        {
            delay(10);
        }

        ESP.restart();
    }
};