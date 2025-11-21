// src/core/EventBus.cpp
#include "EventBus.h"

EventBus* EventBus::instance = nullptr;

EventBus& EventBus::getInstance() {
    if (instance == nullptr) {
        instance = new EventBus();
    }
    return *instance;
}