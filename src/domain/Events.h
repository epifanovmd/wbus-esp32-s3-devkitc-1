// src/domain/Events.h
#pragma once
#include "Entities.h"

String toString(WebastoState state) {
    switch(state) {
        case WebastoState::OFF: return "OFF";
        case WebastoState::READY: return "READY";
        case WebastoState::PARKING_HEAT: return "PARKING_HEAT";
        case WebastoState::VENTILATION: return "VENTILATION";
        case WebastoState::SUPP_HEAT: return "SUPP_HEAT";
        case WebastoState::BOOST: return "BOOST";
        case WebastoState::CIRC_PUMP: return "CIRC_PUMP";
        case WebastoState::STARTUP: return "STARTUP";
        case WebastoState::SHUTDOWN: return "SHUTDOWN";
        case WebastoState::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

String toString(ConnectionState state) {
    switch(state) {
        case ConnectionState::DISCONNECTED: return "DISCONNECTED";
        case ConnectionState::CONNECTING: return "CONNECTING";
        case ConnectionState::CONNECTED: return "CONNECTED";
        case ConnectionState::CONNECTION_FAILED: return "CONNECTION_FAILED";
        default: return "UNKNOWN";
    }
}

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
        json += "\"oldState\":\"" + toString(oldState) + "\",";
        json += "\"newState\":\"" + toString(newState) + "\"";
        json += "}";
        return json;
    }
};

struct ConnectionStateChangedEvent {
    ConnectionState oldState;
    ConnectionState newState;
    
    String toJson() const {
        String json = "{";
        json += "\"oldState\":\"" + toString(oldState) + "\",";
        json += "\"newState\":\"" + toString(newState) + "\"";
        json += "}";
        return json;
    }
};
