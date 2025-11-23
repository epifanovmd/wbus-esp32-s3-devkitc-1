// src/infrastructure/network/WebSocketServer.h
#pragma once
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include "../../interfaces/ISocketServer.h"
#include "../../core/EventBus.h"
#include "../../domain/Events.h"

class WebSocketServer : public ISocketServer
{
private:
    WebSocketsServer webSocket;
    EventBus &eventBus;
    bool enabled = false;
    uint16_t port;

public:
    WebSocketServer(EventBus &bus, uint16_t wsPort = 81)
        : webSocket(wsPort), eventBus(bus), port(wsPort) {}

    bool initialize() override
    {
        webSocket.begin();
        webSocket.onEvent([this](uint8_t num, WStype_t type, uint8_t *payload, size_t length)
                          { handleWebSocketEvent(num, type, payload, length); });

        enabled = true;

        eventBus.subscribe(EventType::HEATER_STATE_CHANGED,
                           [this](const Event &event)
                           {
                               const auto &stateEvent = static_cast<const TypedEvent<HeaterStateChangedEvent> &>(event);
                               broadcastJson(EventType::HEATER_STATE_CHANGED, stateEvent.data.toJson());
                           });

        eventBus.subscribe(EventType::CONNECTION_STATE_CHANGED,
                           [this](const Event &event)
                           {
                               const auto &connectionEvent = static_cast<const TypedEvent<ConnectionStateChangedEvent> &>(event);
                               broadcastJson(EventType::CONNECTION_STATE_CHANGED, connectionEvent.data.toJson());
                           });

        Serial.println("✅ WebSocket Server started on port " + String(port));
        return true;
    }

    void process() override
    {
        webSocket.loop();
    }

    void broadcastJson(EventType eventType, const String &json) override
    {
        if (!enabled)
            return;

        String message = "{";
        message += "\"type\":" + eventBus.getEventTypeString(eventType) + ",";
        message += "\"data\":" + json;
        message += "}";

        webSocket.broadcastTXT(message);
    }

    bool isWebSocketConnected() override
    {
        return webSocket.connectedClients() > 0;
    }

private:
    void handleWebSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
    {
        switch (type)
        {
        case WStype_DISCONNECTED:
            Serial.printf("[%u] Disconnected!\n", num);
            break;
        case WStype_CONNECTED:
        {
            IPAddress ip = webSocket.remoteIP(num);
            Serial.printf("[%u] Connected from %d.%d.%d.%d\n", num, ip[0], ip[1], ip[2], ip[3]);
        }
        break;
        default:
            break;
        }
    }
};