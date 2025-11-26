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

        setupEventHandlers();

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
        message += "\"type\":\"" + eventBus.getEventTypeString(eventType) + "\",";
        message += "\"data\":" + json;
        message += "}";

        webSocket.broadcastTXT(message);
    }

    bool isWebSocketConnected() override
    {
        return webSocket.connectedClients() > 0;
    }

private:
    void setupEventHandlers() {
        // =========================================================================
        // СОБЫТИЯ СОСТОЯНИЯ ПОДКЛЮЧЕНИЯ И СИСТЕМЫ
        // =========================================================================
        
        eventBus.subscribe(EventType::HEATER_STATE_CHANGED,
            [this](const Event& event) {
                const auto& stateEvent = static_cast<const TypedEvent<HeaterStateChangedEvent>&>(event);
                broadcastJson(EventType::HEATER_STATE_CHANGED, stateEvent.data.toJson());
            });

        eventBus.subscribe(EventType::CONNECTION_STATE_CHANGED,
            [this](const Event& event) {
                const auto& connectionEvent = static_cast<const TypedEvent<ConnectionStateChangedEvent>&>(event);
                broadcastJson(EventType::CONNECTION_STATE_CHANGED, connectionEvent.data.toJson());
            });

        eventBus.subscribe(EventType::KEEP_ALLIVE_SENT,
            [this](const Event& event) {
                broadcastJson(EventType::KEEP_ALLIVE_SENT, "{}");
            });

        // =========================================================================
        // СОБЫТИЯ КОМАНД И ОТВЕТОВ
        // =========================================================================
        
        // eventBus.subscribe(EventType::COMMAND_SENT,
        //     [this](const Event& event) {
        //         broadcastJson(EventType::COMMAND_SENT, "\"" + event.source + "\"");
        //     });

        eventBus.subscribe(EventType::COMMAND_SENT_TIMEOUT,
            [this](const Event& event) {
                const auto& timeoutEvent = static_cast<const TypedEvent<ConnectionTimeoutEvent>&>(event);
                broadcastJson(EventType::COMMAND_SENT_TIMEOUT, timeoutEvent.data.toJson());
            });

        eventBus.subscribe(EventType::COMMAND_SENT_ERRROR,
            [this](const Event& event) {
                broadcastJson(EventType::COMMAND_SENT_ERRROR, "\"" + event.source + "\"");
            });

        // eventBus.subscribe(EventType::COMMAND_RECEIVED,
        //     [this](const Event& event) {
        //         const auto& receivedEvent = static_cast<const TypedEvent<CommandReceivedEvent>&>(event);
        //         broadcastJson(EventType::COMMAND_RECEIVED, receivedEvent.data.toJson());
        //     });

        // =========================================================================
        // СОБЫТИЯ ПЕРЕХВАТА K-LINE ПАКЕТОВ
        // =========================================================================
        
        eventBus.subscribe(EventType::TX_RECEIVED,
            [this](const Event& event) {
                broadcastJson(EventType::TX_RECEIVED, "\"" + event.source + "\"");
            });

        eventBus.subscribe(EventType::RX_RECEIVED,
            [this](const Event& event) {
                broadcastJson(EventType::RX_RECEIVED, "\"" + event.source + "\"");
            });

        // =========================================================================
        // СОБЫТИЯ ИНФОРМАЦИИ ОБ УСТРОЙСТВЕ
        // =========================================================================
        
        eventBus.subscribe(EventType::WBUS_VERSION,
            [this](const Event& event) {
                broadcastJson(EventType::WBUS_VERSION, "\"" + String(event.source) + "\"");
            });

        eventBus.subscribe(EventType::DEVICE_NAME,
            [this](const Event& event) {
                broadcastJson(EventType::DEVICE_NAME, "\"" + String(event.source) + "\"");
            });

        eventBus.subscribe(EventType::WBUS_CODE,
            [this](const Event& event) {
                const auto& codeEvent = static_cast<const TypedEvent<DecodedWBusCode>&>(event);
                broadcastJson(EventType::WBUS_CODE, codeEvent.data.toJson());
            });

        eventBus.subscribe(EventType::DEVICE_ID,
            [this](const Event& event) {
                broadcastJson(EventType::DEVICE_ID, "\"" + String(event.source) + "\"");
            });

        eventBus.subscribe(EventType::CONTRALLER_MANUFACTURE_DATE,
            [this](const Event& event) {
                const auto& dateEvent = static_cast<const TypedEvent<DecodedManufactureDate>&>(event);
                broadcastJson(EventType::CONTRALLER_MANUFACTURE_DATE, dateEvent.data.toJson());
            });

        eventBus.subscribe(EventType::HEATER_MANUFACTURE_DATE,
            [this](const Event& event) {
                const auto& dateEvent = static_cast<const TypedEvent<DecodedManufactureDate>&>(event);
                broadcastJson(EventType::HEATER_MANUFACTURE_DATE, dateEvent.data.toJson());
            });

        eventBus.subscribe(EventType::CUSTOMER_ID,
            [this](const Event& event) {
                broadcastJson(EventType::CUSTOMER_ID, "\"" + String(event.source) + "\"");
            });

        eventBus.subscribe(EventType::SERIAL_NUMBER,
            [this](const Event& event) {
                broadcastJson(EventType::SERIAL_NUMBER, "\"" + String(event.source) + "\"");
            });

        // =========================================================================
        // СОБЫТИЯ ОШИБОК WEBASTO
        // =========================================================================
        
        eventBus.subscribe(EventType::WBUS_ERRORS,
            [this](const Event& event) {
                const auto& errorsEvent = static_cast<const TypedEvent<ErrorCollection>&>(event);
                broadcastJson(EventType::WBUS_ERRORS, errorsEvent.data.toJson());
            });

        eventBus.subscribe(EventType::WBUS_CLEAR_ERRORS_SUCCESS,
            [this](const Event& event) {
                broadcastJson(EventType::WBUS_CLEAR_ERRORS_SUCCESS, "{\"status\":\"success\"}");
            });

        eventBus.subscribe(EventType::WBUS_CLEAR_ERRORS_FAILED,
            [this](const Event& event) {
                broadcastJson(EventType::WBUS_CLEAR_ERRORS_FAILED, "{\"status\":\"failed\"}");
            });

        // =========================================================================
        // СОБЫТИЯ ДАТЧИКОВ
        // =========================================================================
        
        eventBus.subscribe(EventType::SENSOR_OPERATIONAL_INFO,
            [this](const Event& event) {
                const auto& sensorEvent = static_cast<const TypedEvent<OperationalMeasurements>&>(event);
                broadcastJson(EventType::SENSOR_OPERATIONAL_INFO, sensorEvent.data.toJson());
            });

        eventBus.subscribe(EventType::SENSOR_ON_OFF_FLAGS,
            [this](const Event& event) {
                const auto& flagsEvent = static_cast<const TypedEvent<OnOffFlags>&>(event);
                broadcastJson(EventType::SENSOR_ON_OFF_FLAGS, flagsEvent.data.toJson());
            });

        eventBus.subscribe(EventType::SENSOR_STATUS_FLAGS,
            [this](const Event& event) {
                const auto& statusEvent = static_cast<const TypedEvent<StatusFlags>&>(event);
                broadcastJson(EventType::SENSOR_STATUS_FLAGS, statusEvent.data.toJson());
            });

        eventBus.subscribe(EventType::SENSOR_OPERATIONG_STATE,
            [this](const Event& event) {
                const auto& stateEvent = static_cast<const TypedEvent<OperatingState>&>(event);
                broadcastJson(EventType::SENSOR_OPERATIONG_STATE, stateEvent.data.toJson());
            });

        eventBus.subscribe(EventType::SENSOR_SUBSYSTEM_STATE,
            [this](const Event& event) {
                const auto& subsystemEvent = static_cast<const TypedEvent<SubsystemsStatus>&>(event);
                broadcastJson(EventType::SENSOR_SUBSYSTEM_STATE, subsystemEvent.data.toJson());
            });

        eventBus.subscribe(EventType::FUEL_SETTINGS,
            [this](const Event& event) {
                const auto& fuelEvent = static_cast<const TypedEvent<FuelSettings>&>(event);
                broadcastJson(EventType::FUEL_SETTINGS, fuelEvent.data.toJson());
            });

        // =========================================================================
        // СОБЫТИЯ ТЕСТИРОВАНИЯ КОМПОНЕНТОВ
        // =========================================================================
        
        eventBus.subscribe(EventType::TEST_COMBUSTION_FAN_STARTED,
            [this](const Event& event) {
                broadcastJson(EventType::TEST_COMBUSTION_FAN_STARTED, "{\"component\":\"combustion_fan\",\"status\":\"started\"}");
            });

        eventBus.subscribe(EventType::TEST_COMBUSTION_FAN_FAILED,
            [this](const Event& event) {
                broadcastJson(EventType::TEST_COMBUSTION_FAN_FAILED, "{\"component\":\"combustion_fan\",\"status\":\"failed\"}");
            });

        eventBus.subscribe(EventType::TEST_FUEL_PUMP_STARTED,
            [this](const Event& event) {
                broadcastJson(EventType::TEST_FUEL_PUMP_STARTED, "{\"component\":\"fuel_pump\",\"status\":\"started\"}");
            });

        eventBus.subscribe(EventType::TEST_FUEL_PUMP_FAILED,
            [this](const Event& event) {
                broadcastJson(EventType::TEST_FUEL_PUMP_FAILED, "{\"component\":\"fuel_pump\",\"status\":\"failed\"}");
            });

        eventBus.subscribe(EventType::TEST_GLOW_PLUG_STARTED,
            [this](const Event& event) {
                broadcastJson(EventType::TEST_GLOW_PLUG_STARTED, "{\"component\":\"glow_plug\",\"status\":\"started\"}");
            });

        eventBus.subscribe(EventType::TEST_GLOW_PLUG_FAILED,
            [this](const Event& event) {
                broadcastJson(EventType::TEST_GLOW_PLUG_FAILED, "{\"component\":\"glow_plug\",\"status\":\"failed\"}");
            });

        eventBus.subscribe(EventType::TEST_CIRCULATION_PUMP_STARTED,
            [this](const Event& event) {
                broadcastJson(EventType::TEST_CIRCULATION_PUMP_STARTED, "{\"component\":\"circulation_pump\",\"status\":\"started\"}");
            });

        eventBus.subscribe(EventType::TEST_CIRCULATION_PUMP_FAILED,
            [this](const Event& event) {
                broadcastJson(EventType::TEST_CIRCULATION_PUMP_FAILED, "{\"component\":\"circulation_pump\",\"status\":\"failed\"}");
            });

        eventBus.subscribe(EventType::TEST_VEHICLE_FAN_STARTED,
            [this](const Event& event) {
                broadcastJson(EventType::TEST_VEHICLE_FAN_STARTED, "{\"component\":\"vehicle_fan\",\"status\":\"started\"}");
            });

        eventBus.subscribe(EventType::TEST_VEHICLE_FAN_FAILED,
            [this](const Event& event) {
                broadcastJson(EventType::TEST_VEHICLE_FAN_FAILED, "{\"component\":\"vehicle_fan\",\"status\":\"failed\"}");
            });

        eventBus.subscribe(EventType::TEST_SOLENOID_STARTED,
            [this](const Event& event) {
                broadcastJson(EventType::TEST_SOLENOID_STARTED, "{\"component\":\"solenoid\",\"status\":\"started\"}");
            });

        eventBus.subscribe(EventType::TEST_SOLENOID_FAILED,
            [this](const Event& event) {
                broadcastJson(EventType::TEST_SOLENOID_FAILED, "{\"component\":\"solenoid\",\"status\":\"failed\"}");
            });

        eventBus.subscribe(EventType::TEST_FUEL_PREHEATING_STARTED,
            [this](const Event& event) {
                broadcastJson(EventType::TEST_FUEL_PREHEATING_STARTED, "{\"component\":\"fuel_preheating\",\"status\":\"started\"}");
            });

        eventBus.subscribe(EventType::TEST_FUEL_PREHEATING_FAILED,
            [this](const Event& event) {
                broadcastJson(EventType::TEST_FUEL_PREHEATING_FAILED, "{\"component\":\"fuel_preheating\",\"status\":\"failed\"}");
            });
    }

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
            
            // Отправляем текущее состояние при подключении
            sendInitialState(num);
        }
        break;
        case WStype_TEXT:
            handleWebSocketMessage(num, payload, length);
            break;
        default:
            break;
        }
    }

    void handleWebSocketMessage(uint8_t num, uint8_t *payload, size_t length) {
        String message = String((char*)payload).substring(0, length);
        
        // Здесь можно добавить обработку входящих сообщений от клиента
        // Например, команды управления нагревателем
        
        Serial.printf("[%u] Received: %s\n", num, message.c_str());
        
        // Эхо-ответ для тестирования
        webSocket.sendTXT(num, "{\"type\":\"echo\", \"message\":\"received: " + message + "\"}");
    }

    void sendInitialState(uint8_t clientNum) {
        // Отправляем начальное состояние новому клиенту
        String welcome = "{\"type\":\"welcome\", \"message\":\"Connected to Webasto Controller\"}";
        webSocket.sendTXT(clientNum, welcome);
    }
};