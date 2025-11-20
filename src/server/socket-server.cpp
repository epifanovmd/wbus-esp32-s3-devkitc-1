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
                default: 
                Serial.print(type);
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

    String json;
    serializeJson(doc, json);
    webSocket.broadcastTXT(json);
}

void SocketServer::send(const String &type, const String &data)
{
    if (!enabled)
        return;

    DynamicJsonDocument doc(512);

    doc["type"] = type;
    doc["data"] = serialized(data);

    String json;
    serializeJson(doc, json);
    webSocket.broadcastTXT(json);
}

void SocketServer::sendSystemStatus(const String &type, const String &currentState, const String &prevState)
{
    if (!enabled)
        return;

    DynamicJsonDocument doc(512);

    JsonObject data = doc.createNestedObject("data");

    doc["type"] = type;

    data["state"] = currentState;
    data["prev_state"] = prevState;

    String json;
    serializeJson(doc, json);
    webSocket.broadcastTXT(json);
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