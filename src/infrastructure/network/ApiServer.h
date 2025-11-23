// src/infrastructure/network/ApiServer.h
#pragma once
#include <WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "../../interfaces/IWebServer.h"
#include "../../application/HeaterController.h"
#include "../../application/ErrorsManager.h"

class ApiServer : public IWebServer {
private:
    WebServer server;
    DeviceInfoManager& deviceInfoManager;
    SensorManager& sensorManager;
    ErrorsManager& errorsManager;
    HeaterController& heaterController;
    uint16_t port;
    
public:
    ApiServer(DeviceInfoManager& deviceInfoMngr, SensorManager& sensorMngr, ErrorsManager& errorsMngr, HeaterController& heaterCtrl, uint16_t webPort = 80)
        : deviceInfoManager(deviceInfoMngr)
        , sensorManager(sensorMngr)
        , errorsManager(errorsMngr)
        , heaterController(heaterCtrl)
        , port(webPort) {}
    
    bool initialize() override {
        if (!LittleFS.begin(true)) {
            Serial.println("❌ LittleFS initialization failed");
            return false;
        }
        
        setupEndpoints();
        server.begin();
        
        Serial.println("✅ HTTP Server started on port " + String(port));
        // printAvailableEndpoints();
        return true;
    }
    
    void process() override {
        server.handleClient();
    }

private:
    void setupEndpoints() {
        server.onNotFound([this]() { handleNotFound(); });
    }

    
    
    void handleNotFound() {
        server.send(404, "application/json", 
            "{\"error\":\"not_found\",\"uri\":\"" + server.uri() + "\"}");
    }
};