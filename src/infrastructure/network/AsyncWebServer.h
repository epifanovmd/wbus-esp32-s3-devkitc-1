#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include "./domain/Events.h"
#include "./common/Constants.h"
#include "./OtaHandlers.h"
#include "./WebastoApiHandlers.h"
#include "./SystemHandlers.h"
#include "./EventHandlers.h"
#include "./ConfigApiHandlers.h"
#include "./WebSocketManager.h"
#include "./core/FileSystemManager.h"
#include "./ApiHelpers.h"
#include "../../application/HeaterController.h"
#include "../../application/ErrorsManager.h"
#include "../../application/DeviceInfoManager.h"
#include "../../application/SensorManager.h"

class AsyncApiServer
{
private:
    AsyncWebServer server;
    WebSocketManager webSocketManager;

    FileSystemManager &fsManager;
    ConfigManager &configManager;
    DeviceInfoManager &deviceInfoManager;
    SensorManager &sensorManager;
    ErrorsManager &errorsManager;
    HeaterController &heaterController;
    WebastoApiHandlers webastoApiHandlers;
    OtaHandlers otaHandlers;
    SystemHandlers systemHandlers;
    EventHandlers eventHandlers;
    ConfigApiHandlers configApiHandlers;

public:
    AsyncApiServer(
        FileSystemManager &fsMgr,
        ConfigManager &configMngr,
        DeviceInfoManager &deviceInfoMngr,
        SensorManager &sensorMngr,
        ErrorsManager &errorsMngr,
        HeaterController &heaterCtrl)
        : server(configMngr.getConfig().network.port),
          fsManager(fsMgr),
          configManager(configMngr),
          deviceInfoManager(deviceInfoMngr),
          sensorManager(sensorMngr),
          errorsManager(errorsMngr),
          heaterController(heaterCtrl),
          webastoApiHandlers(server, deviceInfoMngr, sensorMngr, errorsMngr, heaterCtrl),
          systemHandlers(server, configMngr),
          webSocketManager(heaterCtrl),
          eventHandlers(webSocketManager),
          otaHandlers(server, webSocketManager, configMngr, fsManager),
          configApiHandlers(server, configMngr, fsManager)
    {
    }

    bool initialize()
    {
        if (!fsManager.begin())
        {
            Serial.println("❌ LittleFS initialization failed");
            return false;
        }

        if (!fsManager.exists("/index.html"))
        {
            Serial.println("⚠️  index.html not found");
        }

        eventHandlers.setupEventHandlers();
        server.addHandler(&webSocketManager.getWebSocket());

        DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
        DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");

        setupEndpoints();

        server.serveStatic("/", LittleFS, "/")
            .setTryGzipFirst(false)
            .setDefaultFile("index.html")
            .setCacheControl("max-age=3600");

        server.onNotFound([this](AsyncWebServerRequest *request)
                          { handleNotFound(request); });

        server.begin();

        uint16_t port = configManager.getConfig().network.port;
        String portString = port > 0 && port != 80 ? (":" + String(port)) : "";

        Serial.println("📱 Connect to: http://" + WiFi.softAPIP().toString() + portString);
        Serial.println("✅ WebSocket available at ws://" + WiFi.softAPIP().toString() + portString + "/ws");

        ApiHelpers::printAvailableEndpoints();

        return true;
    }

    void process()
    {
        webSocketManager.process();
        otaHandlers.process();
    }

    void broadcastJson(EventType eventType,
                       const String &json)
    {
        webSocketManager.broadcastJson(eventType, json);
    }

    bool isWebSocketConnected()
    {
        return webSocketManager.isConnected();
    }

private:
    void setupEndpoints()
    {
        webastoApiHandlers.setupEndpoints();
        otaHandlers.setupEndpoints();
        systemHandlers.setupEndpoints();
        configApiHandlers.setupEndpoints();
    }
    void handleNotFound(AsyncWebServerRequest *request)
    {
        String path = request->url();
        if (path.startsWith("/api/"))
        {
            ApiHelpers::sendJsonResponse(request,
                                         "{\"error\":\"not_found\",\"endpoint\":\"" + path + "\"}", 404);
        }
        else
        {
            ApiHelpers::sendJsonResponse(request,
                                         "{\"error\":\"not_found\",\"path\":\"" + path + "\"}", 404);
        }
    }
};