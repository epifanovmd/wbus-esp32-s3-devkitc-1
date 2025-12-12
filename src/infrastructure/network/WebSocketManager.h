#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "./domain/Events.h"
#include "../../application/HeaterController.h"
#include "./WebSocketSubscriptionManager.h"

class WebSocketManager
{
private:
    EventBus &eventBus;
    AsyncWebSocket ws;
    HeaterController &heaterController;
    WebSocketSubscriptionManager subscriptionManager;

public:
    WebSocketManager(EventBus &bus, HeaterController &heaterCtrl) : eventBus(bus),
                                                                    ws("/ws"),
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

    void broadcastJsonToClient(EventType eventType, const String &json)
    {
        auto subscribers = subscriptionManager.getEventSubscribers(eventType);

        if (subscribers.empty())
            return;

        String message = createMessage(eventType, json);

        for (auto clientId : subscribers)
        {
            sendToClient(clientId, message);
        }
    }

    void broadcastJson(EventType eventType, const String &json)
    {
        if (ws.count() == 0)
            return;

        String message = createMessage(eventType, json);
        ws.textAll(message);
    }

    void sendToClient(uint32_t clientId, const String &message)
    {
        auto client = ws.client(clientId);
        if (client && client->status() == WS_CONNECTED)
        {
            client->text(message);
        }
        else
        {
            subscriptionManager.unsubscribeAll(clientId);
        }
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
            handleConnect(client);
            break;
        }
        case WS_EVT_DISCONNECT:
            handleDisconnect(client);
            break;
        case WS_EVT_DATA:
            handleWebSocketMessage(client, arg, data, len);
            break;
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
        }
    }

    void handleConnect(AsyncWebSocketClient *client)
    {
        IPAddress ip = client->remoteIP();
        Serial.printf("[WebSocket] Client #%u connected from %s\n",
                      client->id(), ip.toString().c_str());

        // Отправляем приветственное сообщение с доступными событиями
        sendWelcomeMessage(client);
    }

    void handleDisconnect(AsyncWebSocketClient *client)
    {
        Serial.printf("[WebSocket] Client #%u disconnected\n", client->id());
        // Удаляем все подписки клиента
        subscriptionManager.unsubscribeAll(client->id());
    }

    void handleWebSocketMessage(AsyncWebSocketClient *client, void *arg,
                                uint8_t *data, size_t len)
    {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
        {
            data[len] = 0;
            String message = String((char *)data);

            DynamicJsonDocument doc(512);
            DeserializationError error = deserializeJson(doc, message);

            if (error)
            {
                sendError(client, "Invalid JSON");
                return;
            }

            processMessage(client, doc);
        }
    }

    void processMessage(AsyncWebSocketClient *client, DynamicJsonDocument &doc)
    {
        String type = doc["type"] | "";

        if (type == "ping")
        {
            sendPong(client);
        }
        else if (type == "subscribe")
        {
            processSubscribe(client, doc);
        }
        else if (type == "unsubscribe")
        {
            processUnsubscribe(client, doc);
        }
    }

    void processSubscribe(AsyncWebSocketClient *client, DynamicJsonDocument &doc)
    {
        String eventStr = doc["data"] | "";

        if (eventStr.isEmpty())
        {
            sendError(client, "Event not specified");
            return;
        }

        // Конвертируем строку в EventType
        EventType eventType = stringToEventType(eventStr);

        if (eventType == static_cast<EventType>(-1))
        { // Неизвестное событие
            sendError(client, "Unknown event: " + eventStr);
            return;
        }

        subscriptionManager.subscribe(client->id(), eventType);

        DynamicJsonDocument response(256);
        response["type"] = "subscribe_ack";
        response["event"] = eventStr;
        response["success"] = true;

        String json;
        serializeJson(response, json);
        client->text(json);
    }

    void processUnsubscribe(AsyncWebSocketClient *client, DynamicJsonDocument &doc)
    {
        String eventStr = doc["data"] | "";

        if (eventStr == "all")
        {
            subscriptionManager.unsubscribeAll(client->id());

            DynamicJsonDocument response(256);
            response["type"] = "unsubscribe_ack";
            response["event"] = "all";
            response["success"] = true;

            String json;
            serializeJson(response, json);
            client->text(json);
        }
        else if (!eventStr.isEmpty())
        {
            EventType eventType = stringToEventType(eventStr);

            if (eventType != static_cast<EventType>(-1))
            {
                subscriptionManager.unsubscribe(client->id(), eventType);

                DynamicJsonDocument response(256);
                response["type"] = "unsubscribe_ack";
                response["event"] = eventStr;
                response["success"] = true;

                String json;
                serializeJson(response, json);
                client->text(json);
            }
        }
    }

    void sendWelcomeMessage(AsyncWebSocketClient *client)
    {
        DynamicJsonDocument doc(1024);
        doc["type"] = "welcome";
        doc["clientId"] = client->id();
        doc["server"] = "Webasto Controller";

        // Информация о нагревателе
        HeaterStatus status = heaterController.getStatus();
        doc["heaterState"] = status.getStateName();
        doc["connectionState"] = status.getConnectionName();

        // Доступные события
        JsonArray events = doc.createNestedArray("availableEvents");

        auto availableEvents = eventBus.getAllEventStrings();

        for (const auto &event : availableEvents)
        {
            events.add(event);
        }

        String json;
        serializeJson(doc, json);
        client->text(json);
    }

    void sendError(AsyncWebSocketClient *client, const String &message)
    {
        DynamicJsonDocument doc(256);
        doc["type"] = "error";
        doc["message"] = message;

        String json;
        serializeJson(doc, json);
        client->text(json);
    }

    void sendPong(AsyncWebSocketClient *client)
    {
        DynamicJsonDocument doc(128);
        doc["type"] = "pong";

        String json;
        serializeJson(doc, json);
        client->text(json);
    }

    String createMessage(EventType eventType, const String &jsonData)
    {
        DynamicJsonDocument doc(512 + jsonData.length());
        doc["type"] = eventTypeToString(eventType);

        // Парсим jsonData и добавляем как объект data
        DynamicJsonDocument dataDoc(1024);
        deserializeJson(dataDoc, jsonData);
        doc["data"] = dataDoc.as<JsonVariant>();

        String message;
        serializeJson(doc, message);
        return message;
    }

    EventType stringToEventType(const String &str)
    {
        return eventBus.fromString(str);
    }

    String eventTypeToString(EventType type)
    {
        return eventBus.toString(type);
    }
};