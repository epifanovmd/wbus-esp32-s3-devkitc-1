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
        printAvailableEndpoints();
        return true;
    }
    
    void process() override {
        server.handleClient();
    }

private:
    void setupEndpoints() {
        // Статический HTML
        server.on("/", HTTP_GET, [this]() { serveHTML(); });
        
        // API endpoints
        server.on("/api/system/state", HTTP_GET, [this]() { handleGetSystemState(); });
        server.on("/api/sensors/data", HTTP_GET, [this]() { handleGetSensorsData(); });
        
        // Управление
        server.on("/api/control/connect", HTTP_POST, [this]() { handleConnect(); });
        server.on("/api/control/disconnect", HTTP_POST, [this]() { handleDisconnect(); });
        server.on("/api/control/start_parking", HTTP_POST, [this]() { handleStartParkingHeat(); });
        server.on("/api/control/start_ventilation", HTTP_POST, [this]() { handleStartVentilation(); });
        server.on("/api/control/start_supplemental", HTTP_POST, [this]() { handleStartSupplementalHeat(); });
        server.on("/api/control/start_boost", HTTP_POST, [this]() { handleStartBoostMode(); });
        server.on("/api/control/circulation_pump", HTTP_POST, [this]() { handleControlCirculationPump(); });
        server.on("/api/control/stop", HTTP_POST, [this]() { handleStopHeater(); });
        
        server.onNotFound([this]() { handleNotFound(); });
    }
    
    void serveHTML() {
        // Упрощенная HTML страница (полная версия из оригинального кода)
        String html = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Webasto W-Bus</title>
    <style>
        body { font-family: Arial; margin: 20px; background: #f0f0f0; }
        .card { background: white; padding: 20px; margin: 10px; border-radius: 10px; }
        .btn { padding: 10px 15px; margin: 5px; background: #007bff; color: white; border: none; border-radius: 5px; }
    </style>
</head>
<body>
    <h1>🚗 Webasto W-Bus Control</h1>
    
    <div class="card">
        <h3>🎮 Управление</h3>
        <button class="btn" onclick="sendCommand('connect')">Подключиться</button>
        <button class="btn" onclick="sendCommand('start_parking')">Паркинг-нагрев</button>
        <button class="btn" onclick="sendCommand('stop')">Остановить</button>
    </div>
    
    <div id="status" class="card">
        <h3>📊 Статус</h3>
        <div id="status-content">Загрузка...</div>
    </div>
    
    <script>
        async function sendCommand(command) {
            try {
                const response = await fetch('/api/control/' + command, { method: 'POST' });
                const data = await response.json();
                alert(data.message || 'Команда выполнена');
                updateStatus();
            } catch (error) {
                alert('Ошибка: ' + error);
            }
        }
        
        async function updateStatus() {
            try {
                const response = await fetch('/api/system/state');
                const data = await response.json();
                document.getElementById('status-content').innerHTML = 
                    'Состояние: ' + data.heater_state + '<br>' +
                    'Подключение: ' + data.connection_state;
            } catch (error) {
                document.getElementById('status-content').innerHTML = 'Ошибка загрузки статуса';
            }
        }
        
        // Обновляем статус при загрузке и каждые 5 секунд
        updateStatus();
        setInterval(updateStatus, 5000);
    </script>
</body>
</html>
        )rawliteral";
        
        server.send(200, "text/html", html);
    }
    
    void handleGetSystemState() {
        HeaterStatus status = heaterController.getStatus();

        server.send(200, "application/json", status.toJson());
    }
    
    void handleGetSensorsData() {
        OperationalMeasurements data = sensorManager.getOperationalMeasurementsData();

        server.send(200, "application/json", data.toJson());
    }
    
    void handleConnect() {
        // В новой архитектуре подключение управляется через BusManager
        // Здесь просто возвращаем успех
        server.send(200, "application/json", "{\"success\":true,\"message\":\"Connect command received\"}");
    }
    
    void handleDisconnect() {
        server.send(200, "application/json", "{\"success\":true,\"message\":\"Disconnect command received\"}");
    }
    
    void handleStartParkingHeat() {
        int minutes = server.arg("minutes").toInt();
        if (minutes <= 0) minutes = 60;
        
        bool result = heaterController.startParkingHeat(minutes);
        server.send(200, "application/json", 
            "{\"success\":" + String(result ? "true" : "false") + 
            ",\"message\":\"Parking heat started for " + String(minutes) + " minutes\"}");
    }
    
    void handleStartVentilation() {
        int minutes = server.arg("minutes").toInt();
        if (minutes <= 0) minutes = 60;
        
        bool result = heaterController.startVentilation(minutes);
        server.send(200, "application/json",
            "{\"success\":" + String(result ? "true" : "false") +
            ",\"message\":\"Ventilation started for " + String(minutes) + " minutes\"}");
    }
    
    void handleStartSupplementalHeat() {
        int minutes = server.arg("minutes").toInt();
        if (minutes <= 0) minutes = 60;
        
        bool result = heaterController.startSupplementalHeat(minutes);
        server.send(200, "application/json",
            "{\"success\":" + String(result ? "true" : "false") +
            ",\"message\":\"Supplemental heat started for " + String(minutes) + " minutes\"}");
    }
    
    void handleStartBoostMode() {
        int minutes = server.arg("minutes").toInt();
        if (minutes <= 0) minutes = 60;
        
        bool result = heaterController.startBoostMode(minutes);
        server.send(200, "application/json",
            "{\"success\":" + String(result ? "true" : "false") +
            ",\"message\":\"Boost mode started for " + String(minutes) + " minutes\"}");
    }
    
    void handleControlCirculationPump() {
        String enableStr = server.arg("enable");
        bool enable = (enableStr == "true" || enableStr == "1");
        
        bool result = heaterController.controlCirculationPump(enable);
        server.send(200, "application/json",
            "{\"success\":" + String(result ? "true" : "false") +
            ",\"message\":\"Circulation pump " + String(enable ? "enabled" : "disabled") + "\"}");
    }
    
    void handleStopHeater() {
        bool result = heaterController.shutdown();
        server.send(200, "application/json",
            "{\"success\":" + String(result ? "true" : "false") +
            ",\"message\":\"Heater shutdown command sent\"}");
    }
    
    void handleNotFound() {
        server.send(404, "application/json", 
            "{\"error\":\"not_found\",\"uri\":\"" + server.uri() + "\"}");
    }
    
    void printAvailableEndpoints() {
        Serial.println("📋 Available API endpoints:");
        Serial.println("  GET  /api/system/state        - System state");
        Serial.println("  GET  /api/sensors/data        - Sensors data");
        Serial.println("  POST /api/control/connect     - Connect to Webasto");
        Serial.println("  POST /api/control/disconnect  - Disconnect from Webasto");
        Serial.println("  POST /api/control/start_parking - Start parking heat");
        Serial.println("  POST /api/control/start_ventilation - Start ventilation");
        Serial.println("  POST /api/control/start_supplemental - Start supplemental heat");
        Serial.println("  POST /api/control/start_boost - Start boost mode");
        Serial.println("  POST /api/control/circulation_pump - Control circulation pump");
        Serial.println("  POST /api/control/stop        - Stop heater");
    }
};