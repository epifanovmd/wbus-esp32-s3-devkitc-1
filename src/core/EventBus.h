// src/core/EventBus.h
#pragma once
#include <Arduino.h>
#include <functional>
#include <map>
#include <vector>

enum class EventType
{
    CONNECTION_STATE_CHANGED,
    HEATER_STATE_CHANGED,
    KEEP_ALLIVE_SENT,

    // События запуска теста компонентов
    TEST_COMBUSTION_FAN_STARTED,
    TEST_COMBUSTION_FAN_FAILED,
    TEST_FUEL_PUMP_STARTED,
    TEST_FUEL_PUMP_FAILED,
    TEST_GLOW_PLUG_STARTED,
    TEST_GLOW_PLUG_FAILED,
    TEST_CIRCULATION_PUMP_STARTED,
    TEST_CIRCULATION_PUMP_FAILED,
    TEST_VEHICLE_FAN_STARTED,
    TEST_VEHICLE_FAN_FAILED,
    TEST_SOLENOID_STARTED,
    TEST_SOLENOID_FAILED,
    TEST_FUEL_PREHEATING_STARTED,
    TEST_FUEL_PREHEATING_FAILED,

    // События отправки пакетов
    COMMAND_SENT,
    COMMAND_SENT_TIMEOUT,
    COMMAND_SENT_ERRROR,
    COMMAND_RECEIVED,

    // События перехвата пакетов k-line
    TX_RECEIVED,
    RX_RECEIVED,

    // события информации об устройстве
    WBUS_VERSION,
    DEVICE_NAME,
    WBUS_CODE,
    DEVICE_ID,
    CONTRALLER_MANUFACTURE_DATE,
    HEATER_MANUFACTURE_DATE,
    CUSTOMER_ID,
    SERIAL_NUMBER,

    // события ошибок Webasto
    WBUS_ERRORS,
    WBUS_CLEAR_ERRORS_SUCCESS,
    WBUS_CLEAR_ERRORS_FAILED,
    COMMAND_NAK_RESPONSE,

    // события датчиков
    SENSOR_OPERATIONAL_INFO,
    SENSOR_ON_OFF_FLAGS,
    SENSOR_STATUS_FLAGS,
    SENSOR_OPERATING_STATE,
    SENSOR_SUBSYSTEM_STATE,
    FUEL_SETTINGS,
    SENSOR_OPERATING_TIMES,
    FUEL_PREWARMING,
    BURNING_DURATION_STATS,
    START_COUNTERS,
};

struct Event
{
    EventType type;
    String source;

    virtual ~Event() = default;
};

template <typename T>
struct TypedEvent : public Event
{
    T data;

    TypedEvent(EventType eventType, T eventData)
        : data(eventData)
    {
        type = eventType;
    }
};

class EventBus
{
private:
    static EventBus *instance;
    std::map<EventType, std::vector<std::function<void(const Event &)>>> subscribers;

public:
    static EventBus &getInstance();

    void subscribe(EventType type, std::function<void(const Event &)> handler)
    {
        subscribers[type].push_back(handler);
    }

    template <typename T>
    void publish(EventType type, const T data, const String &source = "")
    {
        TypedEvent<T> event(type, data);
        event.source = source;
        publishInternal(event);
    }

    void publish(EventType type, const String &source = "")
    {
        Event event;
        event.type = type;
        event.source = source;
        publishInternal(event);
    }

    String getEventTypeString(EventType type)
    {
        switch (type)
        {
        case EventType::CONNECTION_STATE_CHANGED:
            return "CONNECTION_STATE_CHANGED";
        case EventType::HEATER_STATE_CHANGED:
            return "HEATER_STATE_CHANGED";
        case EventType::KEEP_ALLIVE_SENT:
            return "KEEP_ALLIVE_SENT";

        case EventType::TEST_COMBUSTION_FAN_STARTED:
            return "TEST_COMBUSTION_FAN_STARTED";
        case EventType::TEST_COMBUSTION_FAN_FAILED:
            return "TEST_COMBUSTION_FAN_FAILED";
        case EventType::TEST_FUEL_PUMP_STARTED:
            return "TEST_FUEL_PUMP_STARTED";
        case EventType::TEST_FUEL_PUMP_FAILED:
            return "TEST_FUEL_PUMP_FAILED";
        case EventType::TEST_GLOW_PLUG_STARTED:
            return "TEST_GLOW_PLUG_STARTED";
        case EventType::TEST_GLOW_PLUG_FAILED:
            return "TEST_GLOW_PLUG_FAILED";
        case EventType::TEST_CIRCULATION_PUMP_STARTED:
            return "TEST_CIRCULATION_PUMP_STARTED";
        case EventType::TEST_CIRCULATION_PUMP_FAILED:
            return "TEST_CIRCULATION_PUMP_FAILED";
        case EventType::TEST_VEHICLE_FAN_STARTED:
            return "TEST_VEHICLE_FAN_STARTED";
        case EventType::TEST_VEHICLE_FAN_FAILED:
            return "TEST_VEHICLE_FAN_FAILED";
        case EventType::TEST_SOLENOID_STARTED:
            return "TEST_SOLENOID_STARTED";
        case EventType::TEST_SOLENOID_FAILED:
            return "TEST_SOLENOID_FAILED";
        case EventType::TEST_FUEL_PREHEATING_STARTED:
            return "TEST_FUEL_PREHEATING_STARTED";
        case EventType::TEST_FUEL_PREHEATING_FAILED:
            return "TEST_FUEL_PREHEATING_FAILED";

        case EventType::COMMAND_SENT:
            return "COMMAND_SENT";
        case EventType::COMMAND_SENT_TIMEOUT:
            return "COMMAND_SENT_TIMEOUT";
        case EventType::COMMAND_SENT_ERRROR:
            return "COMMAND_SENT_ERRROR";
        case EventType::COMMAND_RECEIVED:
            return "COMMAND_RECEIVED";

        case EventType::TX_RECEIVED:
            return "TX_RECEIVED";
        case EventType::RX_RECEIVED:
            return "RX_RECEIVED";

        case EventType::WBUS_VERSION:
            return "WBUS_VERSION";
        case EventType::DEVICE_NAME:
            return "DEVICE_NAME";
        case EventType::WBUS_CODE:
            return "WBUS_CODE";
        case EventType::DEVICE_ID:
            return "DEVICE_ID";
        case EventType::CONTRALLER_MANUFACTURE_DATE:
            return "CONTRALLER_MANUFACTURE_DATE";
        case EventType::HEATER_MANUFACTURE_DATE:
            return "HEATER_MANUFACTURE_DATE";
        case EventType::CUSTOMER_ID:
            return "CUSTOMER_ID";
        case EventType::SERIAL_NUMBER:
            return "SERIAL_NUMBER";

        case EventType::WBUS_ERRORS:
            return "WBUS_ERRORS";
        case EventType::WBUS_CLEAR_ERRORS_SUCCESS:
            return "WBUS_CLEAR_ERRORS_SUCCESS";
        case EventType::WBUS_CLEAR_ERRORS_FAILED:
            return "WBUS_CLEAR_ERRORS_FAILED";

        case EventType::SENSOR_OPERATIONAL_INFO:
            return "SENSOR_OPERATIONAL_INFO";
        case EventType::SENSOR_ON_OFF_FLAGS:
            return "SENSOR_ON_OFF_FLAGS";
        case EventType::SENSOR_STATUS_FLAGS:
            return "SENSOR_STATUS_FLAGS";
        case EventType::SENSOR_OPERATING_STATE:
            return "SENSOR_OPERATING_STATE";
        case EventType::SENSOR_SUBSYSTEM_STATE:
            return "SENSOR_SUBSYSTEM_STATE";
        case EventType::FUEL_SETTINGS:
            return "FUEL_SETTINGS";
        case EventType::SENSOR_OPERATING_TIMES:
            return "SENSOR_OPERATING_TIMES";
        case EventType::FUEL_PREWARMING:
            return "FUEL_PREWARMING";
        case EventType::BURNING_DURATION_STATS:
            return "BURNING_DURATION_STATS";
        case EventType::START_COUNTERS:
            return "START_COUNTERS";
        case EventType::COMMAND_NAK_RESPONSE:
            return "COMMAND_NAK_RESPONSE";

        default:
            return "UNKNOWN_EVENT";
        }
    }

private:
    void publishInternal(const Event &event)
    {
        auto it = subscribers.find(event.type);
        if (it != subscribers.end())
        {
            for (auto &handler : it->second)
            {
                handler(event);
            }
        }
    }
};