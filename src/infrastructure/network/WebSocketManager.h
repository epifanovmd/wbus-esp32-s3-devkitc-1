#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "./domain/Events.h"
#include "../../application/HeaterController.h"

class WebSocketManager
{
private:
    AsyncWebSocket ws;
    HeaterController &heaterController;

public:
    WebSocketManager(HeaterController &heaterCtrl) : ws("/ws"),
                                                     heaterController(heaterCtrl)
    {
        ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client,
                          AwsEventType type, void *arg, uint8_t *data, size_t len)
                   { handleWebSocketEvent(server, client, type, arg, data, len); });
    }

    AsyncWebSocket &getWebSocket()
    {
        return ws;
    }

    void process()
    {
        ws.cleanupClients();
    }

    void broadcastJson(EventType eventType,
                       const String &json)
    {
        if (ws.count() == 0)
            return;

        EventBus &eventBus = EventBus::getInstance();
        String eventTypeStr = eventBus.getEventTypeString(eventType);

        String message = "{";
        message += "\"type\":\"" + eventTypeStr + "\",";
        message += "\"data\":" + json;
        message += "}";

        ws.textAll(message);
    }

    bool isConnected()
    {
        return ws.count() > 0;
    }

private:
    void handleWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                              AwsEventType type, void *arg, uint8_t *data, size_t len)
    {
        switch (type)
        {
        case WS_EVT_CONNECT:
        {
            IPAddress ip = client->remoteIP();
            Serial.printf("[WebSocket] Client #%u connected from %d.%d.%d.%d\n",
                          client->id(), ip[0], ip[1], ip[2], ip[3]);
            sendInitialState(client);
            break;
        }
        case WS_EVT_DISCONNECT:
            Serial.printf("[WebSocket] Client #%u disconnected\n", client->id());
            break;
        case WS_EVT_DATA:
            handleWebSocketMessage(client, arg, data, len);
            break;
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
        }
    }

    void handleWebSocketMessage(AsyncWebSocketClient *client, void *arg,
                                uint8_t *data, size_t len)
    {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
        {
            data[len] = 0;
            String message = String((char *)data);

            DynamicJsonDocument doc(128);
            DeserializationError error = deserializeJson(doc, message);

            if (error)
            {
                Serial.printf("[WebSocket] Invalid JSON: %s\n", error.c_str());
                return;
            }

            String type = doc["type"] | "";

            if (type == "ping")
            {
                DynamicJsonDocument pongDoc(128);
                pongDoc["type"] = "pong";
                String pongJson;
                serializeJson(pongDoc, pongJson);
                ws.text(client->id(), pongJson);
            }
        }
    }

    void sendInitialState(AsyncWebSocketClient *client)
    {
        DynamicJsonDocument doc(512);
        doc["type"] = "welcome";
        doc["message"] = "Connected to Webasto Controller";
        doc["server"] = "ESP32 AsyncWebServer";
        doc["version"] = "1.0.0";

        HeaterStatus status = heaterController.getStatus();
        doc["heaterState"] = status.getStateName();
        doc["connectionState"] = status.getConnectionName();

        String json;
        serializeJson(doc, json);
        client->text(json);
    }
};