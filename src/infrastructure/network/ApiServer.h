// src/infrastructure/network/ApiServer.h
#pragma once
#include <WebServer.h>
#include <LittleFS.h>
#include <ArduinoJson.h>
#include "../../interfaces/IWebServer.h"
#include "../../application/HeaterController.h"
#include "../../application/ErrorsManager.h"
#include "../../application/DeviceInfoManager.h"
#include "../../application/SensorManager.h"

class ApiServer : public IWebServer {
private:
    WebServer server;
    DeviceInfoManager& deviceInfoManager;
    SensorManager& sensorManager;
    ErrorsManager& errorsManager;
    HeaterController& heaterController;
    uint16_t port;

    
public:
    ApiServer(DeviceInfoManager& deviceInfoMngr, SensorManager& sensorMngr, 
              ErrorsManager& errorsMngr, HeaterController& heaterCtrl, uint16_t webPort = 80)
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
        printAvailableEndpoints();
        return true;
    }
    
    void process() override {
        server.handleClient();
    }

private:
    void setupEndpoints() {
        // API endpoints - Управление подключением
        server.on("/api/connect", HTTP_POST, [this]() { handleConnect(); });
        server.on("/api/disconnect", HTTP_POST, [this]() { handleDisconnect(); });
        server.on("/api/status", HTTP_GET, [this]() { handleGetStatus(); });
        
        // API endpoints - Основные режимы
        server.on("/api/start/parking", HTTP_POST, [this]() { handleStartParking(); });
        server.on("/api/start/ventilation", HTTP_POST, [this]() { handleStartVentilation(); });
        server.on("/api/start/supplemental", HTTP_POST, [this]() { handleStartSupplemental(); });
        server.on("/api/start/boost", HTTP_POST, [this]() { handleStartBoost(); });
        server.on("/api/control/circulation-pump", HTTP_POST, [this]() { handleControlCirculationPump(); });
        server.on("/api/shutdown", HTTP_POST, [this]() { handleShutdown(); });
        
        // API endpoints - Тестирование компонентов
        server.on("/api/test/combustion-fan", HTTP_POST, [this]() { handleTestCombustionFan(); });
        server.on("/api/test/fuel-pump", HTTP_POST, [this]() { handleTestFuelPump(); });
        server.on("/api/test/glow-plug", HTTP_POST, [this]() { handleTestGlowPlug(); });
        server.on("/api/test/circulation-pump", HTTP_POST, [this]() { handleTestCirculationPump(); });
        server.on("/api/test/vehicle-fan", HTTP_POST, [this]() { handleTestVehicleFan(); });
        server.on("/api/test/solenoid", HTTP_POST, [this]() { handleTestSolenoid(); });
        server.on("/api/test/fuel-preheating", HTTP_POST, [this]() { handleTestFuelPreheating(); });
        
        // API endpoints - Информация
        server.on("/api/device/info", HTTP_GET, [this]() { handleGetDeviceInfo(); });
        server.on("/api/sensors/data", HTTP_GET, [this]() { handleGetSensorsData(); });
        server.on("/api/errors", HTTP_GET, [this]() { handleGetErrors(); });
        server.on("/api/errors/clear", HTTP_POST, [this]() { handleClearErrors(); });
        
        server.onNotFound([this]() { handleFiles(); });
    }
    
   void handleFiles() {
        String path = server.uri();
        
        // Если запрос корневой, отдаем index.html
        if (path == "/") {
            path = "/index.html";
        }
        
        Serial.println("📁 File request: " + path);
        
        // Проверяем существование файла
        if (!LittleFS.exists(path)) {
            Serial.println("❌ File not found: " + path);
            server.send(404, "application/json", 
                "{\"error\":\"not_found\",\"uri\":\"" + server.uri() + "\",\"path\":\"" + path + "\"}");
            return;
        }
        
        // Определяем Content-Type
        String contentType = getContentType(path);
        Serial.println("📄 Content-Type: " + contentType + " for " + path);
        
        // Открываем и отправляем файл
        File file = LittleFS.open(path, "r");
        if (!file) {
            server.send(500, "text/plain", "Error opening file");
            return;
        }
        
        server.streamFile(file, contentType);
        file.close();
    }
    
    // Определение Content-Type по расширению файла
    String getContentType(String filename) {
        if (filename.endsWith(".html")) return "text/html";
        if (filename.endsWith(".css")) return "text/css";
        if (filename.endsWith(".js")) return "application/javascript";
        if (filename.endsWith(".json")) return "application/json";
        if (filename.endsWith(".png")) return "image/png";
        if (filename.endsWith(".jpg") || filename.endsWith(".jpeg")) return "image/jpeg";
        if (filename.endsWith(".gif")) return "image/gif";
        if (filename.endsWith(".ico")) return "image/x-icon";
        if (filename.endsWith(".svg")) return "image/svg+xml";
        if (filename.endsWith(".woff")) return "font/woff";
        if (filename.endsWith(".woff2")) return "font/woff2";
        if (filename.endsWith(".ttf")) return "font/ttf";
        if (filename.endsWith(".txt")) return "text/plain";
        return "application/octet-stream";
    }
    
    // =========================================================================
    // ОБРАБОТЧИКИ API ENDPOINTS
    // =========================================================================
    
    void handleConnect() {
        heaterController.connect();
        server.send(200, "application/json", "{\"status\":\"connecting\"}");
    }
    
    void handleDisconnect() {
        heaterController.disconnect();
        server.send(200, "application/json", "{\"status\":\"disconnected\"}");
    }
    
    void handleGetStatus() {
        HeaterStatus status = heaterController.getStatus();
        
        DynamicJsonDocument doc(512);
        doc["heater_state"] = status.getStateName();
        doc["connection_state"] = status.getConnectionName();
        doc["is_connected"] = status.isConnected();
        
        String json;
        serializeJson(doc, json);
        server.send(200, "application/json", json);
    }
    
    void handleStartParking() {
        int minutes = server.hasArg("minutes") ? server.arg("minutes").toInt() : 60;
        heaterController.startParkingHeat(minutes);
        server.send(200, "application/json", "{\"status\":\"started\",\"mode\":\"parking\",\"minutes\":" + String(minutes) + "}");
    }
    
    void handleStartVentilation() {
        int minutes = server.hasArg("minutes") ? server.arg("minutes").toInt() : 60;
        heaterController.startVentilation(minutes);
        server.send(200, "application/json", "{\"status\":\"started\",\"mode\":\"ventilation\",\"minutes\":" + String(minutes) + "}");
    }
    
    void handleStartSupplemental() {
        int minutes = server.hasArg("minutes") ? server.arg("minutes").toInt() : 60;
        heaterController.startSupplementalHeat(minutes);
        server.send(200, "application/json", "{\"status\":\"started\",\"mode\":\"supplemental\",\"minutes\":" + String(minutes) + "}");
    }
    
    void handleStartBoost() {
        int minutes = server.hasArg("minutes") ? server.arg("minutes").toInt() : 60;
        heaterController.startBoostMode(minutes);
        server.send(200, "application/json", "{\"status\":\"started\",\"mode\":\"boost\",\"minutes\":" + String(minutes) + "}");
    }
    
    void handleControlCirculationPump() {
        bool enable = server.hasArg("enable") ? server.arg("enable") == "true" : false;
        heaterController.controlCirculationPump(enable);
        server.send(200, "application/json", "{\"status\":\"updated\",\"pump_enabled\":" + String(enable ? "true" : "false") + "}");
    }
    
    void handleShutdown() {
        heaterController.shutdown();
        server.send(200, "application/json", "{\"status\":\"shutdown\"}");
    }
    
    // Тестирование компонентов
    void handleTestCombustionFan() {
        int seconds = server.hasArg("seconds") ? server.arg("seconds").toInt() : 10;
        int power = server.hasArg("power") ? server.arg("power").toInt() : 50;
        heaterController.testCombustionFan(seconds, power);
        server.send(200, "application/json", "{\"status\":\"testing\",\"component\":\"combustion_fan\",\"seconds\":" + String(seconds) + ",\"power\":" + String(power) + "}");
    }
    
    void handleTestFuelPump() {
        int seconds = server.hasArg("seconds") ? server.arg("seconds").toInt() : 10;
        int frequency = server.hasArg("frequency") ? server.arg("frequency").toInt() : 10;
        heaterController.testFuelPump(seconds, frequency);
        server.send(200, "application/json", "{\"status\":\"testing\",\"component\":\"fuel_pump\",\"seconds\":" + String(seconds) + ",\"frequency\":" + String(frequency) + "}");
    }
    
    void handleTestGlowPlug() {
        int seconds = server.hasArg("seconds") ? server.arg("seconds").toInt() : 10;
        int power = server.hasArg("power") ? server.arg("power").toInt() : 75;
        heaterController.testGlowPlug(seconds, power);
        server.send(200, "application/json", "{\"status\":\"testing\",\"component\":\"glow_plug\",\"seconds\":" + String(seconds) + ",\"power\":" + String(power) + "}");
    }
    
    void handleTestCirculationPump() {
        int seconds = server.hasArg("seconds") ? server.arg("seconds").toInt() : 10;
        int power = server.hasArg("power") ? server.arg("power").toInt() : 100;
        heaterController.testCirculationPump(seconds, power);
        server.send(200, "application/json", "{\"status\":\"testing\",\"component\":\"circulation_pump\",\"seconds\":" + String(seconds) + ",\"power\":" + String(power) + "}");
    }
    
    void handleTestVehicleFan() {
        int seconds = server.hasArg("seconds") ? server.arg("seconds").toInt() : 10;
        heaterController.testVehicleFan(seconds);
        server.send(200, "application/json", "{\"status\":\"testing\",\"component\":\"vehicle_fan\",\"seconds\":" + String(seconds) + "}");
    }
    
    void handleTestSolenoid() {
        int seconds = server.hasArg("seconds") ? server.arg("seconds").toInt() : 10;
        heaterController.testSolenoidValve(seconds);
        server.send(200, "application/json", "{\"status\":\"testing\",\"component\":\"solenoid\",\"seconds\":" + String(seconds) + "}");
    }
    
    void handleTestFuelPreheating() {
        int seconds = server.hasArg("seconds") ? server.arg("seconds").toInt() : 10;
        int power = server.hasArg("power") ? server.arg("power").toInt() : 50;
        heaterController.testFuelPreheating(seconds, power);
        server.send(200, "application/json", "{\"status\":\"testing\",\"component\":\"fuel_preheating\",\"seconds\":" + String(seconds) + ",\"power\":" + String(power) + "}");
    }
    
    void handleGetDeviceInfo() {
        String json = deviceInfoManager.getDeviceInfoJson();
        server.send(200, "application/json", json);
    }
    
    void handleGetSensorsData() {
        String json = sensorManager.getAllSensorsJson();
        server.send(200, "application/json", json);
    }
    
    void handleGetErrors() {
        String json = errorsManager.getErrorsJson();
        server.send(200, "application/json", json);
    }
    
    void handleClearErrors() {
        heaterController.breakIfNeeded();
        errorsManager.resetErrors();
        server.send(200, "application/json", "{\"status\":\"cleared\"}");
    }
    
    void handleNotFound() {
        server.send(404, "application/json", 
            "{\"error\":\"not_found\",\"uri\":\"" + server.uri() + "\"}");
    }
    
    void printAvailableEndpoints() {
        Serial.println();
        Serial.println("🌐 Available API Endpoints:");
        Serial.println("  GET  /                    - Web interface");
        Serial.println("  GET  /assets/*            - Static files (CSS, JS, images)");
        Serial.println("  POST /api/connect         - Connect to Webasto");
        Serial.println("  POST /api/disconnect      - Disconnect from Webasto");
        Serial.println("  GET  /api/status          - Get system status");
        Serial.println("  POST /api/start/parking   - Start parking heat");
        Serial.println("  POST /api/start/ventilation - Start ventilation");
        Serial.println("  POST /api/start/supplemental - Start supplemental heat");
        Serial.println("  POST /api/start/boost     - Start boost mode");
        Serial.println("  POST /api/shutdown        - Shutdown heater");
        Serial.println("  GET  /api/device/info     - Get device information");
        Serial.println("  GET  /api/sensors/data    - Get sensors data");
        Serial.println("  GET  /api/errors          - Get errors list");
        Serial.println("  POST /api/errors/clear    - Clear errors");
        Serial.println();
    }
};