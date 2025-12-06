#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <LittleFS.h>
#include "./common/Version.h"

class SystemHandlers
{
private:
    AsyncWebServer &server;

    // Форматирование частоты процессора
    String formatFrequency(uint32_t frequency)
    {
        if (frequency < 1000)
            return String(frequency) + " MHz";
        else
            return String(frequency / 1000.0, 1) + " GHz";
    }

    // Режим WiFi в текстовом виде
    String getWiFiModeString(wifi_mode_t mode)
    {
        switch (mode)
        {
        case WIFI_MODE_NULL:
            return "NULL";
        case WIFI_MODE_STA:
            return "Station";
        case WIFI_MODE_AP:
            return "Access Point";
        case WIFI_MODE_APSTA:
            return "AP+Station";
        default:
            return "Unknown";
        }
    }

    // Информация о чипе
    String getChipInfo()
    {
        String info = ESP.getChipModel();
        info += " (Rev ";
        info += String(ESP.getChipRevision());
        info += ")";
        return info;
    }

    // Полная информация о системе
    void addSystemInfo(DynamicJsonDocument &doc)
    {
        JsonObject system = doc.createNestedObject("system");

        // Основная информация о чипе
        system["chipModel"] = ESP.getChipModel();
        system["chipRevision"] = ESP.getChipRevision();
        system["chipCores"] = ESP.getChipCores();
        system["chipDescription"] = getChipInfo();

        // ID устройства
        char chipId[13];
        snprintf(chipId, sizeof(chipId), "%08X", (uint32_t)ESP.getEfuseMac());
        system["chipId"] = chipId;

        // Частота процессора
        system["cpuFrequencyMhz"] = ESP.getCpuFreqMHz();
        system["cpuFrequencyFormatted"] = formatFrequency(ESP.getCpuFreqMHz());

        // Версии
        system["sdkVersion"] = ESP.getSdkVersion();
        system["firmwareVersion"] = FIRMWARE_VERSION;
        system["compileDate"] = __DATE__;
        system["compileTime"] = __TIME__;

        // Информация о памяти
        JsonObject memory = doc.createNestedObject("memory");

        // Heap память
        size_t heapFree = ESP.getFreeHeap();
        size_t heapTotal = ESP.getHeapSize();
        size_t heapUsed = heapTotal - heapFree;
        float heapUsagePercent = (heapTotal > 0) ? (heapUsed * 100.0) / heapTotal : 0;

        memory["heap"]["total"] = heapTotal;
        memory["heap"]["free"] = heapFree;
        memory["heap"]["used"] = heapUsed;
        memory["heap"]["totalFormatted"] = String(heapTotal / 1024.0, 1) + " KB";
        memory["heap"]["freeFormatted"] = String(heapFree / 1024.0, 1) + " KB";
        memory["heap"]["usedFormatted"] = String(heapUsed / 1024.0, 1) + " KB";
        memory["heap"]["usagePercent"] = String(heapUsagePercent, 1) + "%";

        // PSRAM (если есть)
        if (ESP.getPsramSize() > 0)
        {
            size_t psramFree = ESP.getFreePsram();
            size_t psramTotal = ESP.getPsramSize();
            size_t psramUsed = psramTotal - psramFree;
            float psramUsagePercent = (psramTotal > 0) ? (psramUsed * 100.0) / psramTotal : 0;

            memory["psram"]["total"] = psramTotal;
            memory["psram"]["free"] = psramFree;
            memory["psram"]["used"] = psramUsed;
            memory["psram"]["totalFormatted"] = String(psramTotal / 1024.0 / 1024.0, 1) + " MB";
            memory["psram"]["freeFormatted"] = String(psramFree / 1024.0 / 1024.0, 1) + " MB";
            memory["psram"]["usedFormatted"] = String(psramUsed / 1024.0 / 1024.0, 1) + " MB";
            memory["psram"]["usagePercent"] = String(psramUsagePercent, 1) + "%";
        }

        // Информация о хранилище
        JsonObject storage = doc.createNestedObject("storage");

        size_t flashSize = ESP.getFlashChipSize();
        size_t sketchSize = ESP.getSketchSize();
        size_t freeSketchSpace = ESP.getFreeSketchSpace();
        size_t usedBySketch = sketchSize;

        storage["flashTotal"] = flashSize;
        storage["flashTotalFormatted"] = String(flashSize / 1024.0 / 1024.0, 1) + " MB";
        storage["sketchSize"] = sketchSize;
        storage["sketchSizeFormatted"] = String(sketchSize / 1024.0, 1) + " KB";
        storage["freeForOTA"] = freeSketchSpace;
        storage["freeForOTAFormatted"] = String(freeSketchSpace / 1024.0, 1) + " KB";
        storage["maxOTASize"] = freeSketchSpace - 0x1000;
        storage["maxOTASizeFormatted"] = String((freeSketchSpace - 0x1000) / 1024.0, 1) + " KB";

        // LittleFS информация
        if (LittleFS.begin(true))
        {
            size_t totalBytes = LittleFS.totalBytes();
            size_t usedBytes = LittleFS.usedBytes();
            size_t freeBytes = totalBytes - usedBytes;
            float littlefsUsagePercent = (totalBytes > 0) ? (usedBytes * 100.0) / totalBytes : 0;

            storage["littlefs"]["total"] = totalBytes;
            storage["littlefs"]["used"] = usedBytes;
            storage["littlefs"]["free"] = freeBytes;
            storage["littlefs"]["totalFormatted"] = String(totalBytes / 1024.0, 1) + " KB";
            storage["littlefs"]["usedFormatted"] = String(usedBytes / 1024.0, 1) + " KB";
            storage["littlefs"]["freeFormatted"] = String(freeBytes / 1024.0, 1) + " KB";
            storage["littlefs"]["usagePercent"] = String(littlefsUsagePercent, 1) + "%";
            LittleFS.end();
        }

        // Информация о WiFi
        JsonObject wifi = doc.createNestedObject("wifi");

        wifi_mode_t mode = WiFi.getMode();
        wifi["mode"] = getWiFiModeString(mode);
        wifi["hostname"] = WiFi.getHostname();

        // Access Point информация
        if (mode == WIFI_MODE_AP || mode == WIFI_MODE_APSTA)
        {
            JsonObject ap = wifi.createNestedObject("accessPoint");
            ap["ip"] = WiFi.softAPIP().toString();
            ap["mac"] = WiFi.softAPmacAddress();
            ap["connectedClients"] = WiFi.softAPgetStationNum();
        }

        // Station информация
        if (mode == WIFI_MODE_STA || mode == WIFI_MODE_APSTA)
        {
            JsonObject sta = wifi.createNestedObject("station");
            sta["status"] = (WiFi.status() == WL_CONNECTED) ? "connected" : "disconnected";

            if (WiFi.status() == WL_CONNECTED)
            {
                sta["ssid"] = WiFi.SSID();
                sta["ip"] = WiFi.localIP().toString();
                sta["gateway"] = WiFi.gatewayIP().toString();
                sta["subnet"] = WiFi.subnetMask().toString();
                sta["dns"] = WiFi.dnsIP().toString();
                sta["mac"] = WiFi.macAddress();
                sta["rssi"] = WiFi.RSSI();
                sta["rssiFormatted"] = String(WiFi.RSSI()) + " dBm";
                sta["bssid"] = WiFi.BSSIDstr();
                sta["channel"] = WiFi.channel();
            }
        }

        // Сетевая статистика
        JsonObject network = doc.createNestedObject("network");
        network["localIP"] = WiFi.localIP().toString();
        network["macAddress"] = WiFi.macAddress();

        // Uptime системы
        JsonObject uptime = doc.createNestedObject("uptime");
        unsigned long seconds = millis() / 1000;
        unsigned long minutes = seconds / 60;
        unsigned long hours = minutes / 60;
        unsigned long days = hours / 24;

        uptime["milliseconds"] = millis();
        uptime["formatted"] = String(days) + "d " +
                              String(hours % 24) + "h " +
                              String(minutes % 60) + "m " +
                              String(seconds % 60) + "s";
    }

    // Отправка JSON ответа
    void sendJsonResponse(AsyncWebServerRequest *request, DynamicJsonDocument &doc)
    {
        String json;
        serializeJson(doc, json);

        AsyncWebServerResponse *response = request->beginResponse(200, "application/json", json);
        response->addHeader("Access-Control-Allow-Origin", "*");
        response->addHeader("Cache-Control", "no-cache, no-store, must-revalidate");
        response->addHeader("Pragma", "no-cache");
        response->addHeader("Expires", "0");

        request->send(response);
    }

public:
    SystemHandlers(AsyncWebServer &serv) : server(serv) {}

    void setupEndpoints()
    {
        // Полная информация о системе
        server.on("/api/system/info", HTTP_GET,
                  [this](AsyncWebServerRequest *request)
                  {
                      handleSystemInfo(request);
                  });

        // Перезагрузка системы
        server.on("/api/system/restart", HTTP_POST,
                  [this](AsyncWebServerRequest *request)
                  {
                      handleSystemRestart(request);
                  });
    }

    // Обработчик получения полной информации
    void handleSystemInfo(AsyncWebServerRequest *request)
    {
        DynamicJsonDocument doc(4096);
        doc["status"] = "ok";
        doc["timestamp"] = millis();

        addSystemInfo(doc);

        sendJsonResponse(request, doc);
    }

    // Обработчик перезагрузки
    void handleSystemRestart(AsyncWebServerRequest *request)
    {
        DynamicJsonDocument doc(256);
        doc["status"] = "ok";
        doc["message"] = "System will restart in 1 second";
        doc["timestamp"] = millis();

        sendJsonResponse(request, doc);

        // Логируем и даем время на отправку ответа
        Serial.println("🔄 System restart requested via API");
        delay(100);

        // Перезагрузка
        ESP.restart();
    }
};