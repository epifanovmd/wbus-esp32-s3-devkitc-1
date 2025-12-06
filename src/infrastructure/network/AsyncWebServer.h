#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "./domain/Events.h"
#include "./common/Constants.h"
#include "./OtaHandlers.h"
#include "./WebastoApiHandlers.h"
#include "./SystemHendlers.h"
#include "./EventHandlers.h"
#include "./WebSocketManager.h"
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
    DeviceInfoManager &deviceInfoManager;
    SensorManager &sensorManager;
    ErrorsManager &errorsManager;
    HeaterController &heaterController;
    WebastoApiHandlers webastoApiHandlers;
    OtaHandlers otaHandlers;
    SystemHandlers systemHandlers;
    EventHandlers eventHandlers;
    uint16_t port;
    bool enabled = false;

public:
    AsyncApiServer(DeviceInfoManager &deviceInfoMngr, SensorManager &sensorMngr,
                   ErrorsManager &errorsMngr, HeaterController &heaterCtrl,
                   uint16_t serverPort = 80)
        : server(serverPort),
          webSocketManager(heaterCtrl),
          deviceInfoManager(deviceInfoMngr),
          sensorManager(sensorMngr),
          errorsManager(errorsMngr),
          heaterController(heaterCtrl),
          webastoApiHandlers(server, deviceInfoMngr, sensorMngr, errorsMngr, heaterCtrl),
          otaHandlers(server),
          systemHandlers(server),
          eventHandlers(webSocketManager),
          port(serverPort) {}

    bool initialize()
    {
        if (!LittleFS.begin(true))
        {
            Serial.println("❌ LittleFS initialization failed");
            return false;
        }

        if (!LittleFS.exists("/index.html"))
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
        enabled = true;

        Serial.println("✅ Async WebServer started on port " + String(port));
        Serial.println("✅ WebSocket available at ws://" + WiFi.softAPIP().toString() + "/ws");
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