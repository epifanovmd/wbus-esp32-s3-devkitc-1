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

    FUEL_CIRCULATION,

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
    WBUS_DETAILS_ERROR,
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

    // Ota events
    OTA_PROGRESS,

    APP_CONFIG_UPDATE
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
    EventBus() = default;
    ~EventBus() = default;

    EventBus(const EventBus &) = delete;
    EventBus &operator=(const EventBus &) = delete;

    std::map<EventType, String> typeToStringMap;
    std::map<String, EventType> stringToTypeMap;

    std::map<EventType, std::vector<std::function<void(const Event &)>>> subscribers;

public:
    static EventBus &getInstance()
    {
        static EventBus instance;
        return instance;
    }

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

    String toString(EventType type)
    {
        initializeEventMap();
        auto it = typeToStringMap.find(type);
        return it != typeToStringMap.end() ? it->second : "UNKNOWN_EVENT";
    }

    EventType fromString(const String &str)
    {
        initializeEventMap();
        auto it = stringToTypeMap.find(str);
        return it != stringToTypeMap.end() ? it->second : static_cast<EventType>(0);
    }

    std::vector<String> getAllEventStrings()
    {
        initializeEventMap();
        std::vector<String> result;
        result.reserve(stringToTypeMap.size());

        for (const auto &pair : stringToTypeMap)
        {
            result.push_back(pair.first);
        }

        return result;
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

    void initializeEventMap()
    {
        if (!typeToStringMap.empty() && !stringToTypeMap.empty())
        {
            return;
        }

        registerEvent(EventType::CONNECTION_STATE_CHANGED, "CONNECTION_STATE_CHANGED");
        registerEvent(EventType::HEATER_STATE_CHANGED, "HEATER_STATE_CHANGED");
        registerEvent(EventType::KEEP_ALLIVE_SENT, "KEEP_ALLIVE_SENT");
        registerEvent(EventType::FUEL_CIRCULATION, "FUEL_CIRCULATION");
        registerEvent(EventType::TEST_COMBUSTION_FAN_STARTED, "TEST_COMBUSTION_FAN_STARTED");
        registerEvent(EventType::TEST_COMBUSTION_FAN_FAILED, "TEST_COMBUSTION_FAN_FAILED");
        registerEvent(EventType::TEST_FUEL_PUMP_STARTED, "TEST_FUEL_PUMP_STARTED");
        registerEvent(EventType::TEST_FUEL_PUMP_FAILED, "TEST_FUEL_PUMP_FAILED");
        registerEvent(EventType::TEST_GLOW_PLUG_STARTED, "TEST_GLOW_PLUG_STARTED");
        registerEvent(EventType::TEST_GLOW_PLUG_FAILED, "TEST_GLOW_PLUG_FAILED");
        registerEvent(EventType::TEST_CIRCULATION_PUMP_STARTED, "TEST_CIRCULATION_PUMP_STARTED");
        registerEvent(EventType::TEST_CIRCULATION_PUMP_FAILED, "TEST_CIRCULATION_PUMP_FAILED");
        registerEvent(EventType::TEST_VEHICLE_FAN_STARTED, "TEST_VEHICLE_FAN_STARTED");
        registerEvent(EventType::TEST_VEHICLE_FAN_FAILED, "TEST_VEHICLE_FAN_FAILED");
        registerEvent(EventType::TEST_SOLENOID_STARTED, "TEST_SOLENOID_STARTED");
        registerEvent(EventType::TEST_SOLENOID_FAILED, "TEST_SOLENOID_FAILED");
        registerEvent(EventType::TEST_FUEL_PREHEATING_STARTED, "TEST_FUEL_PREHEATING_STARTED");
        registerEvent(EventType::TEST_FUEL_PREHEATING_FAILED, "TEST_FUEL_PREHEATING_FAILED");
        registerEvent(EventType::COMMAND_SENT, "COMMAND_SENT");
        registerEvent(EventType::COMMAND_SENT_TIMEOUT, "COMMAND_SENT_TIMEOUT");
        registerEvent(EventType::COMMAND_SENT_ERRROR, "COMMAND_SENT_ERRROR");
        registerEvent(EventType::COMMAND_RECEIVED, "COMMAND_RECEIVED");
        registerEvent(EventType::TX_RECEIVED, "TX_RECEIVED");
        registerEvent(EventType::RX_RECEIVED, "RX_RECEIVED");
        registerEvent(EventType::WBUS_VERSION, "WBUS_VERSION");
        registerEvent(EventType::DEVICE_NAME, "DEVICE_NAME");
        registerEvent(EventType::WBUS_CODE, "WBUS_CODE");
        registerEvent(EventType::DEVICE_ID, "DEVICE_ID");
        registerEvent(EventType::CONTRALLER_MANUFACTURE_DATE, "CONTRALLER_MANUFACTURE_DATE");
        registerEvent(EventType::HEATER_MANUFACTURE_DATE, "HEATER_MANUFACTURE_DATE");
        registerEvent(EventType::CUSTOMER_ID, "CUSTOMER_ID");
        registerEvent(EventType::SERIAL_NUMBER, "SERIAL_NUMBER");
        registerEvent(EventType::WBUS_ERRORS, "WBUS_ERRORS");
        registerEvent(EventType::WBUS_DETAILS_ERROR, "WBUS_DETAILS_ERROR");
        registerEvent(EventType::COMMAND_NAK_RESPONSE, "COMMAND_NAK_RESPONSE");
        registerEvent(EventType::SENSOR_OPERATIONAL_INFO, "SENSOR_OPERATIONAL_INFO");
        registerEvent(EventType::SENSOR_ON_OFF_FLAGS, "SENSOR_ON_OFF_FLAGS");
        registerEvent(EventType::SENSOR_STATUS_FLAGS, "SENSOR_STATUS_FLAGS");
        registerEvent(EventType::SENSOR_OPERATING_STATE, "SENSOR_OPERATING_STATE");
        registerEvent(EventType::SENSOR_SUBSYSTEM_STATE, "SENSOR_SUBSYSTEM_STATE");
        registerEvent(EventType::FUEL_SETTINGS, "FUEL_SETTINGS");
        registerEvent(EventType::SENSOR_OPERATING_TIMES, "SENSOR_OPERATING_TIMES");
        registerEvent(EventType::FUEL_PREWARMING, "FUEL_PREWARMING");
        registerEvent(EventType::BURNING_DURATION_STATS, "BURNING_DURATION_STATS");
        registerEvent(EventType::START_COUNTERS, "START_COUNTERS");
        registerEvent(EventType::OTA_PROGRESS, "OTA_PROGRESS");
        registerEvent(EventType::APP_CONFIG_UPDATE, "APP_CONFIG_UPDATE");
    }

    void registerEvent(EventType type, const String &str)
    {
        typeToStringMap[type] = str;
        stringToTypeMap[str] = type;
    }
};