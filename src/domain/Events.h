// src/domain/Events.h
#pragma once
#include "Entities.h"

struct ConnectionTimeoutEvent {
    int retrie;
    String tx;
    
    String toJson() const {
        String json = "{";
        json += "\"retrie\":" + String(retrie) + ",";
        json += "\"tx\":\"" + tx + "\"";
        json += "}";
        return json;
    }
};

struct CommandReceivedEvent {
    String tx;
    String rx;
    
    String toJson() const {
        String json = "{";
        json += "\"tx\":\"" + tx + "\",";
        json += "\"rx\":\"" + rx + "\"";
        json += "}";
        return json;
    }
};

struct HeaterStateChangedEvent {
    WebastoState oldState;
    WebastoState newState;
    
    String toJson() const {
        String json = "{";
        json += "\"oldState\":\"" + HeaterStatus::getStateName(oldState) + "\",";
        json += "\"newState\":\"" + HeaterStatus::getStateName(newState) + "\"";
        json += "}";
        return json;
    }
};

struct ConnectionStateChangedEvent {
    ConnectionState oldState;
    ConnectionState newState;
    
    String toJson() const {
        String json = "{";
        json += "\"oldState\":\"" + HeaterStatus::getConnectionName(oldState) + "\",";
        json += "\"newState\":\"" + HeaterStatus::getConnectionName(newState) + "\"";
        json += "}";
        return json;
    }
};
