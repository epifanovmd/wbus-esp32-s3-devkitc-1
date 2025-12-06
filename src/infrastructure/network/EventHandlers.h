#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "./domain/Events.h"
#include "./WebSocketManager.h"

class EventHandlers
{
private:
    WebSocketManager &webSocketManager;

public:
    EventHandlers(WebSocketManager &wsMngr) : webSocketManager(wsMngr) {}

    void broadcastJson(EventType eventType,
                       const String &json)
    {
        webSocketManager.broadcastJson(eventType, json);
    }

    bool isWebSocketConnected()
    {
        return webSocketManager.isConnected();
    }

    void setupEventHandlers()
    {
        EventBus &eventBus = EventBus::getInstance();

        eventBus.subscribe(EventType::COMMAND_NAK_RESPONSE,
                           [this](const Event &event)
                           {
                               const auto &nakEvent = static_cast<
                                   const TypedEvent<NakResponseEvent> &>(event);
                               broadcastJson(EventType::COMMAND_NAK_RESPONSE,
                                             nakEvent.data.toJson());
                           });

        eventBus.subscribe(EventType::HEATER_STATE_CHANGED,
                           [this](const Event &event)
                           {
                               const auto &stateEvent = static_cast<
                                   const TypedEvent<HeaterStateChangedEvent> &>(event);
                               broadcastJson(EventType::HEATER_STATE_CHANGED,
                                             stateEvent.data.toJson());
                           });

        eventBus.subscribe(EventType::CONNECTION_STATE_CHANGED,
                           [this](const Event &event)
                           {
                               const auto &connectionEvent = static_cast<
                                   const TypedEvent<ConnectionStateChangedEvent> &>(event);
                               broadcastJson(EventType::CONNECTION_STATE_CHANGED,
                                             connectionEvent.data.toJson());
                           });

        eventBus.subscribe(EventType::KEEP_ALLIVE_SENT,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::KEEP_ALLIVE_SENT, "{}");
                           });

        eventBus.subscribe(EventType::COMMAND_SENT_TIMEOUT,
                           [this](const Event &event)
                           {
                               const auto &timeoutEvent = static_cast<
                                   const TypedEvent<ConnectionTimeoutEvent> &>(event);
                               broadcastJson(EventType::COMMAND_SENT_TIMEOUT,
                                             timeoutEvent.data.toJson());
                           });

        eventBus.subscribe(EventType::COMMAND_SENT_ERRROR,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::COMMAND_SENT_ERRROR,
                                             "\"" + event.source + "\"");
                           });

        eventBus.subscribe(EventType::TX_RECEIVED,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::TX_RECEIVED,
                                             "\"" + event.source + "\"");
                           });

        eventBus.subscribe(EventType::RX_RECEIVED,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::RX_RECEIVED,
                                             "\"" + event.source + "\"");
                           });

        eventBus.subscribe(EventType::COMMAND_RECEIVED,
                           [this](const Event &event)
                           {
                               const auto &cmdEvent = static_cast<
                                   const TypedEvent<CommandReceivedEvent> &>(event);
                               broadcastJson(EventType::COMMAND_RECEIVED,
                                             cmdEvent.data.toJson());
                           });

        eventBus.subscribe(EventType::WBUS_VERSION,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::WBUS_VERSION,
                                             "\"" + String(event.source) + "\"");
                           });

        eventBus.subscribe(EventType::DEVICE_NAME,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::DEVICE_NAME,
                                             "\"" + String(event.source) + "\"");
                           });

        eventBus.subscribe(EventType::WBUS_CODE,
                           [this](const Event &event)
                           {
                               const auto &codeEvent = static_cast<
                                   const TypedEvent<DecodedWBusCode> &>(event);
                               broadcastJson(EventType::WBUS_CODE,
                                             codeEvent.data.toJson());
                           });

        eventBus.subscribe(EventType::DEVICE_ID,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::DEVICE_ID,
                                             "\"" + String(event.source) + "\"");
                           });

        eventBus.subscribe(EventType::CONTRALLER_MANUFACTURE_DATE,
                           [this](const Event &event)
                           {
                               const auto &dateEvent = static_cast<
                                   const TypedEvent<DecodedManufactureDate> &>(event);
                               broadcastJson(EventType::CONTRALLER_MANUFACTURE_DATE,
                                             dateEvent.data.toJson());
                           });

        eventBus.subscribe(EventType::HEATER_MANUFACTURE_DATE,
                           [this](const Event &event)
                           {
                               const auto &dateEvent = static_cast<
                                   const TypedEvent<DecodedManufactureDate> &>(event);
                               broadcastJson(EventType::HEATER_MANUFACTURE_DATE,
                                             dateEvent.data.toJson());
                           });

        eventBus.subscribe(EventType::CUSTOMER_ID,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::CUSTOMER_ID,
                                             "\"" + String(event.source) + "\"");
                           });

        eventBus.subscribe(EventType::SERIAL_NUMBER,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::SERIAL_NUMBER,
                                             "\"" + String(event.source) + "\"");
                           });

        eventBus.subscribe(EventType::WBUS_ERRORS,
                           [this](const Event &event)
                           {
                               const auto &errorsEvent = static_cast<
                                   const TypedEvent<ErrorCollection> &>(event);
                               broadcastJson(EventType::WBUS_ERRORS,
                                             errorsEvent.data.toJson());
                           });

        eventBus.subscribe(EventType::WBUS_DETAILS_ERROR,
                           [this](const Event &event)
                           {
                               const auto &detailsErrorEvent = static_cast<
                                   const TypedEvent<ErrorDetails> &>(event);
                               broadcastJson(EventType::WBUS_DETAILS_ERROR,
                                             detailsErrorEvent.data.toJson());
                           });

        eventBus.subscribe(EventType::SENSOR_OPERATIONAL_INFO,
                           [this](const Event &event)
                           {
                               const auto &sensorEvent = static_cast<
                                   const TypedEvent<OperationalMeasurements> &>(event);
                               broadcastJson(EventType::SENSOR_OPERATIONAL_INFO,
                                             sensorEvent.data.toJson());
                           });

        eventBus.subscribe(EventType::SENSOR_ON_OFF_FLAGS,
                           [this](const Event &event)
                           {
                               const auto &flagsEvent = static_cast<
                                   const TypedEvent<OnOffFlags> &>(event);
                               broadcastJson(EventType::SENSOR_ON_OFF_FLAGS,
                                             flagsEvent.data.toJson());
                           });

        eventBus.subscribe(EventType::SENSOR_STATUS_FLAGS,
                           [this](const Event &event)
                           {
                               const auto &statusEvent = static_cast<
                                   const TypedEvent<StatusFlags> &>(event);
                               broadcastJson(EventType::SENSOR_STATUS_FLAGS,
                                             statusEvent.data.toJson());
                           });

        eventBus.subscribe(EventType::SENSOR_OPERATING_STATE,
                           [this](const Event &event)
                           {
                               const auto &stateEvent = static_cast<
                                   const TypedEvent<OperatingState> &>(event);
                               broadcastJson(EventType::SENSOR_OPERATING_STATE,
                                             stateEvent.data.toJson());
                           });

        eventBus.subscribe(EventType::SENSOR_SUBSYSTEM_STATE,
                           [this](const Event &event)
                           {
                               const auto &subsystemEvent = static_cast<
                                   const TypedEvent<SubsystemsStatus> &>(event);
                               broadcastJson(EventType::SENSOR_SUBSYSTEM_STATE,
                                             subsystemEvent.data.toJson());
                           });

        eventBus.subscribe(EventType::FUEL_SETTINGS,
                           [this](const Event &event)
                           {
                               const auto &fuelEvent = static_cast<
                                   const TypedEvent<FuelSettings> &>(event);
                               broadcastJson(EventType::FUEL_SETTINGS,
                                             fuelEvent.data.toJson());
                           });

        eventBus.subscribe(EventType::SENSOR_OPERATING_TIMES,
                           [this](const Event &event)
                           {
                               const auto &operatingTimes = static_cast<
                                   const TypedEvent<OperatingTimes> &>(event);
                               broadcastJson(EventType::SENSOR_OPERATING_TIMES,
                                             operatingTimes.data.toJson());
                           });

        eventBus.subscribe(EventType::FUEL_PREWARMING,
                           [this](const Event &event)
                           {
                               const auto &fuelPrewarming = static_cast<
                                   const TypedEvent<FuelPrewarming> &>(event);
                               broadcastJson(EventType::FUEL_PREWARMING,
                                             fuelPrewarming.data.toJson());
                           });

        eventBus.subscribe(EventType::BURNING_DURATION_STATS,
                           [this](const Event &event)
                           {
                               const auto &burningDurationEvent = static_cast<
                                   const TypedEvent<BurningDuration> &>(event);
                               broadcastJson(EventType::BURNING_DURATION_STATS,
                                             burningDurationEvent.data.toJson());
                           });

        eventBus.subscribe(EventType::START_COUNTERS,
                           [this](const Event &event)
                           {
                               const auto &startCounters = static_cast<
                                   const TypedEvent<StartCounters> &>(event);
                               broadcastJson(EventType::START_COUNTERS,
                                             startCounters.data.toJson());
                           });

        eventBus.subscribe(EventType::TEST_COMBUSTION_FAN_STARTED,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::TEST_COMBUSTION_FAN_STARTED,
                                             "{\"component\":\"combustionFan\",\"status\":\"started\"}");
                           });

        eventBus.subscribe(EventType::TEST_COMBUSTION_FAN_FAILED,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::TEST_COMBUSTION_FAN_FAILED,
                                             "{\"component\":\"combustionFan\",\"status\":\"failed\"}");
                           });

        eventBus.subscribe(EventType::TEST_FUEL_PUMP_STARTED,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::TEST_FUEL_PUMP_STARTED,
                                             "{\"component\":\"fuelPump\",\"status\":\"started\"}");
                           });

        eventBus.subscribe(EventType::TEST_FUEL_PUMP_FAILED,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::TEST_FUEL_PUMP_FAILED,
                                             "{\"component\":\"fuelPump\",\"status\":\"failed\"}");
                           });

        eventBus.subscribe(EventType::TEST_GLOW_PLUG_STARTED,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::TEST_GLOW_PLUG_STARTED,
                                             "{\"component\":\"glowPlug\",\"status\":\"started\"}");
                           });

        eventBus.subscribe(EventType::TEST_GLOW_PLUG_FAILED,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::TEST_GLOW_PLUG_FAILED,
                                             "{\"component\":\"glowPlug\",\"status\":\"failed\"}");
                           });

        eventBus.subscribe(EventType::TEST_CIRCULATION_PUMP_STARTED,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::TEST_CIRCULATION_PUMP_STARTED,
                                             "{\"component\":\"circulationPump\",\"status\":\"started\"}");
                           });

        eventBus.subscribe(EventType::TEST_CIRCULATION_PUMP_FAILED,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::TEST_CIRCULATION_PUMP_FAILED,
                                             "{\"component\":\"circulationPump\",\"status\":\"failed\"}");
                           });

        eventBus.subscribe(EventType::TEST_VEHICLE_FAN_STARTED,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::TEST_VEHICLE_FAN_STARTED,
                                             "{\"component\":\"vehicleFan\",\"status\":\"started\"}");
                           });

        eventBus.subscribe(EventType::TEST_VEHICLE_FAN_FAILED,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::TEST_VEHICLE_FAN_FAILED,
                                             "{\"component\":\"vehicleFan\",\"status\":\"failed\"}");
                           });

        eventBus.subscribe(EventType::TEST_SOLENOID_STARTED,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::TEST_SOLENOID_STARTED,
                                             "{\"component\":\"solenoid\",\"status\":\"started\"}");
                           });

        eventBus.subscribe(EventType::TEST_SOLENOID_FAILED,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::TEST_SOLENOID_FAILED,
                                             "{\"component\":\"solenoid\",\"status\":\"failed\"}");
                           });

        eventBus.subscribe(EventType::TEST_FUEL_PREHEATING_STARTED,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::TEST_FUEL_PREHEATING_STARTED,
                                             "{\"component\":\"fuelPreheating\",\"status\":\"started\"}");
                           });

        eventBus.subscribe(EventType::TEST_FUEL_PREHEATING_FAILED,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::TEST_FUEL_PREHEATING_FAILED,
                                             "{\"component\":\"fuelPreheating\",\"status\":\"failed\"}");
                           });
    }
};