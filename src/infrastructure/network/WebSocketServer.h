// src/infrastructure/network/WebSocketServer.h
#pragma once
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include "../../interfaces/ISocketServer.h"
#include "../../core/EventBus.h"
#include "../../domain/Events.h"

class WebSocketServer : public ISocketServer {
private:
    WebSocketsServer webSocket;
    EventBus& eventBus;
    bool enabled = false;
    uint16_t port;
    
public:
    WebSocketServer(EventBus& bus, uint16_t wsPort = 81) 
        : webSocket(wsPort), eventBus(bus), port(wsPort) {}
    
    bool initialize() override {
        webSocket.begin();
        webSocket.onEvent([this](uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
            handleWebSocketEvent(num, type, payload, length);
        });
        
        enabled = true;
        
        eventBus.subscribe(EventType::SENSOR_DATA_UPDATED,
            [this](const Event& event) {
                handleSensorDataUpdate(event);
            });
            
        eventBus.subscribe(EventType::HEATER_STATE_CHANGED,
            [this](const Event& event) {
                handleHeaterStateChange(event);
            });
            
        eventBus.subscribe(EventType::CONNECTION_STATE_CHANGED,
            [this](const Event& event) {
                handleConnectionStateChange(event);
            });
        
        Serial.println("✅ WebSocket Server started on port " + String(port));
        return true;
    }
    
    void process() override {
        webSocket.loop();
    }
    
    void broadcastSensorData(const String& json) override {
        if (!enabled) return;
        
        DynamicJsonDocument doc(512);
        doc["type"] = "sensor_data";
        doc["data"] = serialized(json);
        
        String message;
        serializeJson(doc, message);
        webSocket.broadcastTXT(message);
    }
    
    void broadcastHeaterStatus(const String& json) override {
        if (!enabled) return;
        
        DynamicJsonDocument doc(512);
        doc["type"] = "heater_status";
        doc["data"] = serialized(json);
        
        String message;
        serializeJson(doc, message);
        webSocket.broadcastTXT(message);
    }
    
    void broadcastSystemStatus(const String& json) override {
        if (!enabled) return;
        
        DynamicJsonDocument doc(512);
        doc["type"] = "system_status";
        doc["data"] = serialized(json);
        
        String message;
        serializeJson(doc, message);
        webSocket.broadcastTXT(message);
    }
    
    bool isWebSocketConnected() override {
        // ИСПРАВЛЕНО: убираем const для вызова non-const метода
        return webSocket.connectedClients() > 0;
    }

private:
    void handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
        switch (type) {
            case WStype_DISCONNECTED:
                Serial.printf("[%u] Disconnected!\n", num);
                break;
            case WStype_CONNECTED:
                {
                    IPAddress ip = webSocket.remoteIP(num);
                    Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
                    sendInitialData(num);
                }
                break;
            case WStype_TEXT:
                {
                    String message = String((char*)payload);
                    handleWebSocketMessage(num, message);
                }
                break;
            default:
                break;
        }
    }
    
    void sendInitialData(uint8_t clientNum) {
        DynamicJsonDocument doc(512);
        doc["type"] = "connection";
        doc["message"] = "WebSocket connected";
        doc["timestamp"] = millis();
        
        String message;
        serializeJson(doc, message);
        webSocket.sendTXT(clientNum, message);
    }
    
    void handleWebSocketMessage(uint8_t clientNum, const String& message) {
        DynamicJsonDocument doc(256);
        DeserializationError error = deserializeJson(doc, message);
        
        if (error) {
            Serial.println("❌ WebSocket JSON parse error: " + String(error.c_str()));
            return;
        }
        
        String command = doc["command"] | "";
        if (command == "enable_logging") {
            enabled = true;
            sendInfo(clientNum, "WebSocket logging enabled");
        } else if (command == "disable_logging") {
            enabled = false;
            sendInfo(clientNum, "WebSocket logging disabled");
        }
    }
    
    void sendInfo(uint8_t clientNum, const String& info) {
        DynamicJsonDocument doc(256);
        doc["type"] = "info";
        doc["message"] = info;
        
        String message;
        serializeJson(doc, message);
        webSocket.sendTXT(clientNum, message);
    }
    
    void handleSensorDataUpdate(const Event& event) {
        if (!enabled) return;
        
        const auto& sensorEvent = static_cast<const TypedEvent<SensorDataUpdatedEvent>&>(event);
        String json = sensorEvent.data.data.toJson();
        broadcastSensorData(json);
    }
    
    void handleHeaterStateChange(const Event& event) {
        if (!enabled) return;
        
        const auto& heaterEvent = static_cast<const TypedEvent<HeaterStateChangedEvent>&>(event);
        
        DynamicJsonDocument doc(512);
        doc["old_state"] = static_cast<int>(heaterEvent.data.oldState);
        doc["new_state"] = static_cast<int>(heaterEvent.data.newState);
        doc["reason"] = heaterEvent.data.reason;
        
        String json;
        serializeJson(doc, json);
        broadcastHeaterStatus(json);
    }
    
    void handleConnectionStateChange(const Event& event) {
        if (!enabled) return;
        
        const auto& connectionEvent = static_cast<const TypedEvent<ConnectionStateChangedEvent>&>(event);
        
        DynamicJsonDocument doc(512);
        doc["old_state"] = static_cast<int>(connectionEvent.data.oldState);
        doc["new_state"] = static_cast<int>(connectionEvent.data.newState);
        doc["reason"] = connectionEvent.data.reason;
        
        String json;
        serializeJson(doc, json);
        broadcastSystemStatus(json);
    }
};