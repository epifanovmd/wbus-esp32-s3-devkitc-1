#include "socket-server.h"
#include <WiFi.h>

SocketServer socketServer;

SocketServer::SocketServer() : webSocket(81) {}

void SocketServer::begin()
{
    webSocket.begin();
    webSocket.onEvent([this](uint8_t num, WStype_t type, uint8_t *payload, size_t length)
                      {
        switch(type) {
            case WStype_DISCONNECTED:
                Serial.printf("[%u] Disconnected!\n", num);
                break;
            case WStype_CONNECTED:
                {
                    IPAddress ip = webSocket.remoteIP(num);
                    Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);
                }
                break;
            case WStype_TEXT:
                {
                    String message = String((char*)payload);
                    Serial.printf("[%u] Received: %s\n", num, message.c_str());
                    
                    // Обработка команд от клиента
                    DynamicJsonDocument doc(1024);
                    deserializeJson(doc, message);
                    String command = doc["command"];
                    
                    if (command == "enable") {
                        enable();
                    } else if (command == "disable") {
                        disable();
                    }
                }
                break;
        } });

    Serial.println("WebSocket Server started on port 81");
    enabled = true;
}

void SocketServer::loop()
{
    webSocket.loop();
}

void SocketServer::sendRx(const String &data)
{
    if (!enabled)
        return;

    DynamicJsonDocument doc(1024);
    doc["type"] = "rx";
    doc["data"] = data;
    doc["timestamp"] = getTimestamp();
    doc["direction"] = "in";

    String json;
    serializeJson(doc, json);
    webSocket.broadcastTXT(json);
}

void SocketServer::sendTx(const String &data)
{
    if (!enabled)
        return;

    DynamicJsonDocument doc(1024);
    doc["type"] = "tx";
    doc["data"] = data;
    doc["timestamp"] = getTimestamp();
    doc["direction"] = "out";

    String json;
    serializeJson(doc, json);
    webSocket.broadcastTXT(json);
}

void SocketServer::sendInfo(const String &message)
{
    if (!enabled)
        return;

    DynamicJsonDocument doc(512);
    doc["type"] = "info";
    doc["message"] = message;
    doc["timestamp"] = getTimestamp();

    String json;
    serializeJson(doc, json);
    webSocket.broadcastTXT(json);
}

void SocketServer::sendError(const String &message)
{
    if (!enabled)
        return;

    DynamicJsonDocument doc(512);
    doc["type"] = "error";
    doc["message"] = message;
    doc["timestamp"] = getTimestamp();

    String json;
    serializeJson(doc, json);
    webSocket.broadcastTXT(json);
}

void SocketServer::sendSensorData(const String &sensorName, const String &value)
{
    if (!enabled)
        return;

    DynamicJsonDocument doc(512);
    doc["type"] = "sensor";
    doc["sensor"] = sensorName;
    doc["value"] = value;
    doc["timestamp"] = getTimestamp();

    String json;
    serializeJson(doc, json);
    webSocket.broadcastTXT(json);
}

void SocketServer::sendErrorData(const String &errorCode, const String &description)
{
    if (!enabled)
        return;

    DynamicJsonDocument doc(512);
    doc["type"] = "error_data";
    doc["code"] = errorCode;
    doc["description"] = description;
    doc["timestamp"] = getTimestamp();

    String json;
    serializeJson(doc, json);
    webSocket.broadcastTXT(json);
}

void SocketServer::sendSystemStatus(const String &status)
{
    if (!enabled)
        return;

    DynamicJsonDocument doc(512);
    doc["type"] = "status";
    doc["status"] = status;
    doc["timestamp"] = getTimestamp();

    String json;
    serializeJson(doc, json);
    webSocket.broadcastTXT(json);
}

String SocketServer::getTimestamp()
{
    unsigned long ms = millis();
    unsigned long seconds = ms / 1000;
    unsigned long minutes = seconds / 60;
    unsigned long hours = minutes / 60;

    char timestamp[20];
    snprintf(timestamp, sizeof(timestamp), "%02lu:%02lu:%02lu.%03lu",
             hours % 24, minutes % 60, seconds % 60, ms % 1000);
    return String(timestamp);
}

void SocketServer::enable()
{
    enabled = true;
    sendInfo("WebSocket logging enabled");
}

void SocketServer::disable()
{
    sendInfo("WebSocket logging disabled");
    enabled = false;
}