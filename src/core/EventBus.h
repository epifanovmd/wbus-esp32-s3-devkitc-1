// src/core/EventBus.h
#pragma once
#include <Arduino.h>
#include <functional>
#include <map>
#include <vector>

enum class EventType {
    SENSOR_DATA_UPDATED,
    HEATER_STATE_CHANGED,
    CONNECTION_STATE_CHANGED,
    COMMAND_SENT,
    COMMAND_RECEIVED,
    ERROR_OCCURRED,
    WEBSOCKET_MESSAGE,
    // Новые события
    DEVICE_INFO_UPDATED,
    ERRORS_UPDATED,
    OPERATIONAL_DATA_UPDATED,
    FUEL_SETTINGS_UPDATED,
    ON_OFF_FLAGS_UPDATED,
    STATUS_FLAGS_UPDATED,
    OPERATING_STATE_UPDATED,
    SUBSYSTEMS_STATUS_UPDATED
};

struct Event {
    EventType type;
    String source;
    
    virtual ~Event() = default;
};

template<typename T>
struct TypedEvent : public Event {
    T data;
    
    TypedEvent(EventType eventType, T eventData)
        : data(eventData) {
        type = eventType;
    }
};

class EventBus {
private:
    static EventBus* instance;
    std::map<EventType, std::vector<std::function<void(const Event&)>>> subscribers;

public:
    static EventBus& getInstance();
    
    void subscribe(EventType type, std::function<void(const Event&)> handler) {
        subscribers[type].push_back(handler);
    }
    
    template<typename T>
    void publish(EventType type, const T data, const String& source = "") {
        TypedEvent<T> event(type, data);
        event.source = source;
        publishInternal(event);
    }
    
    void publish(EventType type, const String& source = "") {
        Event event;
        event.type = type;
        event.source = source;
        publishInternal(event);
    }

private:
    void publishInternal(const Event& event) {
        auto it = subscribers.find(event.type);
        if (it != subscribers.end()) {
            for (auto& handler : it->second) {
                handler(event);
            }
        }
    }
};