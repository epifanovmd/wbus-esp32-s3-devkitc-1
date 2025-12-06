#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <LittleFS.h>
#include "./domain/Events.h"
#include "./ApiHelpers.h"
#include "./common/Version.h"

class SystemHandlers
{
private:
    AsyncWebServer &server;

    // Вспомогательные методы для форматирования
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

    String formatFrequency(uint32_t frequency)
    {
        if (frequency < 1000)
            return String(frequency) + " MHz";
        else
            return String(frequency / 1000.0, 1) + " GHz";
    }

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

    String getChipInfo()
    {
        String info = ESP.getChipModel();
        info += " (Rev ";
        info += String(ESP.getChipRevision());
        info += ")";
        return info;
    }

    void addMemoryInfo(DynamicJsonDocument &doc)
    {
        JsonObject memory = doc.createNestedObject("memory");

        size_t heapFree = ESP.getFreeHeap();
        size_t heapTotal = ESP.getHeapSize();
        size_t heapUsed = heapTotal - heapFree;

        memory["heapTotal"] = formatBytes(heapTotal);
        memory["heapFree"] = formatBytes(heapFree);
        memory["heapUsed"] = formatBytes(heapUsed);
        memory["heapUsagePercent"] = (heapTotal > 0) ? String((heapUsed * 100) / heapTotal) + "%" : "N/A";

        if (ESP.getPsramSize() > 0)
        {
            JsonObject psram = memory.createNestedObject("psram");
            size_t psramFree = ESP.getFreePsram();
            size_t psramTotal = ESP.getPsramSize();
            size_t psramUsed = psramTotal - psramFree;

            psram["total"] = formatBytes(psramTotal);
            psram["free"] = formatBytes(psramFree);
            psram["used"] = formatBytes(psramUsed);
            psram["usagePercent"] = (psramTotal > 0) ? String((psramUsed * 100) / psramTotal) + "%" : "N/A";
        }
    }

    void addStorageInfo(DynamicJsonDocument &doc)
    {
        JsonObject storage = doc.createNestedObject("storage");

        size_t flashSize = ESP.getFlashChipSize();
        size_t sketchSize = ESP.getSketchSize();
        size_t freeSketchSpace = ESP.getFreeSketchSpace();
        size_t usedBySketch = sketchSize;

        storage["flashTotal"] = formatBytes(flashSize);
        storage["sketchSize"] = formatBytes(sketchSize);
        storage["freeForOTA"] = formatBytes(freeSketchSpace);
        storage["maxOTASize"] = formatBytes(freeSketchSpace - 0x1000);

        // LittleFS информация
        if (LittleFS.begin(true))
        {
            size_t totalBytes = LittleFS.totalBytes();
            size_t usedBytes = LittleFS.usedBytes();
            storage["littlefsTotal"] = formatBytes(totalBytes);
            storage["littlefsUsed"] = formatBytes(usedBytes);
            storage["littlefsFree"] = formatBytes(totalBytes - usedBytes);
            storage["littlefsUsagePercent"] = (totalBytes > 0) ? String((usedBytes * 100) / totalBytes) + "%" : "N/A";
        }
    }

    void addWiFiInfo(DynamicJsonDocument &doc)
    {
        JsonObject wifi = doc.createNestedObject("wifi");

        wifi_mode_t mode = WiFi.getMode();
        wifi["mode"] = getWiFiModeString(mode);

        if (mode == WIFI_MODE_AP || mode == WIFI_MODE_APSTA)
        {
            JsonObject ap = wifi.createNestedObject("accessPoint");
            ap["ip"] = WiFi.softAPIP().toString();
            ap["mac"] = WiFi.softAPmacAddress();
            ap["connectedClients"] = WiFi.softAPgetStationNum();
        }

        if (mode == WIFI_MODE_STA || mode == WIFI_MODE_APSTA)
        {
            JsonObject sta = wifi.createNestedObject("station");
            sta["status"] = (WiFi.status() == WL_CONNECTED) ? "Connected" : "Disconnected";
            if (WiFi.status() == WL_CONNECTED)
            {
                sta["ip"] = WiFi.localIP().toString();
                sta["gateway"] = WiFi.gatewayIP().toString();
                sta["subnet"] = WiFi.subnetMask().toString();
                sta["mac"] = WiFi.macAddress();
                sta["rssi"] = String(WiFi.RSSI()) + " dBm";
                sta["ssid"] = WiFi.SSID();
                sta["bssid"] = WiFi.BSSIDstr();
                sta["channel"] = String(WiFi.channel());
            }
        }
    }

    void addSystemInfo(DynamicJsonDocument &doc)
    {
        JsonObject system = doc.createNestedObject("system");

        system["chip"] = getChipInfo();
        system["cpuFrequency"] = formatFrequency(ESP.getCpuFreqMHz());
        system["sdkVersion"] = ESP.getSdkVersion();
        system["firmwareVersion"] = FIRMWARE_VERSION;

        // Время работы
        uint32_t uptime = millis() / 1000;
        uint32_t days = uptime / 86400;
        uint32_t hours = (uptime % 86400) / 3600;
        uint32_t minutes = (uptime % 3600) / 60;
        uint32_t seconds = uptime % 60;

        char uptimeStr[64];
        snprintf(uptimeStr, sizeof(uptimeStr), "%ud %02uh %02um %02us",
                 days, hours, minutes, seconds);
        system["uptime"] = uptimeStr;

// Температура (если доступно)
#ifdef TEMP_SENSOR
        system["temperature"] = String(temperatureRead(), 1) + "°C";
#endif
    }

public:
    SystemHandlers(AsyncWebServer &serv) : server(serv) {}

    void setupEndpoints()
    {
        server.on("/api/system/info", HTTP_GET,
                  [this](AsyncWebServerRequest *request)
                  {
                      handleSystemInfo(request);
                  });

        server.on("/api/system/info/minimal", HTTP_GET,
                  [this](AsyncWebServerRequest *request)
                  {
                      handleSystemInfoMinimal(request);
                  });

        server.on("/api/system/info/detailed", HTTP_GET,
                  [this](AsyncWebServerRequest *request)
                  {
                      handleSystemInfoDetailed(request);
                  });

        server.on("/api/system/restart", HTTP_POST,
                  [this](AsyncWebServerRequest *request)
                  {
                      handleSystemRestart(request);
                  });
    }

    // =========================================================================
    // ОБРАБОТЧИКИ HTTP ЗАПРОСОВ
    // =========================================================================

    void handleSystemInfo(AsyncWebServerRequest *request)
    {
        DynamicJsonDocument doc(1024);

        // Основная информация
        addSystemInfo(doc);
        addMemoryInfo(doc);
        addStorageInfo(doc);
        addWiFiInfo(doc);

        ApiHelpers::sendJsonDocument(request, doc);
    }

    void handleSystemInfoMinimal(AsyncWebServerRequest *request)
    {
        DynamicJsonDocument doc(512);

        doc["chip"] = getChipInfo();
        doc["firmwareVersion"] = FIRMWARE_VERSION;

        // Время работы
        uint32_t uptime = millis() / 1000;
        char uptimeStr[32];
        snprintf(uptimeStr, sizeof(uptimeStr), "%uh %02um",
                 uptime / 3600, (uptime % 3600) / 60);
        doc["uptime"] = uptimeStr;

        // Память
        size_t heapFree = ESP.getFreeHeap();
        size_t heapTotal = ESP.getHeapSize();
        doc["heap"] = formatBytes(heapFree) + " / " + formatBytes(heapTotal);
        doc["heapUsage"] = String(((heapTotal - heapFree) * 100) / heapTotal) + "%";

        // WiFi
        wifi_mode_t mode = WiFi.getMode();
        doc["wifiMode"] = getWiFiModeString(mode);

        if (mode == WIFI_MODE_AP || mode == WIFI_MODE_APSTA)
        {
            doc["apIp"] = WiFi.softAPIP().toString();
            doc["connectedClients"] = WiFi.softAPgetStationNum();
        }

        ApiHelpers::sendJsonDocument(request, doc);
    }

    void handleSystemInfoDetailed(AsyncWebServerRequest *request)
    {
        DynamicJsonDocument doc(4096);

        // Подробная системная информация
        JsonObject system = doc.createNestedObject("system");
        system["chipModel"] = ESP.getChipModel();
        system["chipRevision"] = ESP.getChipRevision();
        system["chipCores"] = ESP.getChipCores();
        system["cpuFrequencyMhz"] = ESP.getCpuFreqMHz();
        system["cpuFrequencyFormatted"] = formatFrequency(ESP.getCpuFreqMHz());
        system["sdkVersion"] = ESP.getSdkVersion();
        system["firmwareVersion"] = FIRMWARE_VERSION;
        system["compileDate"] = __DATE__;
        system["compileTime"] = __TIME__;

        // ID устройства
        char chipId[13];
        snprintf(chipId, sizeof(chipId), "%08X", (uint32_t)ESP.getEfuseMac());
        system["chipId"] = chipId;

        // Время работы
        uint32_t uptime = millis() / 1000;
        system["uptimeSeconds"] = uptime;
        system["uptimeMilliseconds"] = millis();

        uint32_t days = uptime / 86400;
        uint32_t hours = (uptime % 86400) / 3600;
        uint32_t minutes = (uptime % 3600) / 60;
        uint32_t seconds = uptime % 60;
        char uptimeStr[64];
        snprintf(uptimeStr, sizeof(uptimeStr), "%u days, %02u:%02u:%02u",
                 days, hours, minutes, seconds);
        system["uptimeFormatted"] = uptimeStr;

        // Подробная информация о памяти
        addMemoryInfo(doc);

        // Подробная информация о хранилище
        addStorageInfo(doc);

        // Подробная информация о WiFi
        addWiFiInfo(doc);

        // Сетевая информация
        JsonObject network = doc.createNestedObject("network");
        network["hostname"] = WiFi.getHostname();
        network["dns"] = WiFi.dnsIP().toString();

        ApiHelpers::sendJsonDocument(request, doc);
    }

    void handleSystemRestart(AsyncWebServerRequest *request)
    {
        DynamicJsonDocument doc(128);
        doc["message"] = "System will restart in 1 second";

        ApiHelpers::sendJsonDocument(request, doc);

        // Даем время на отправку ответа
        delay(100);

        // Регистрируем задачу на перезагрузку
        Serial.println("🔄 System restart requested via API");
        ESP.restart();
    }
};