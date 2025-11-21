// src/interfaces/ISocketServer.h
#pragma once
#include <Arduino.h>
#include <functional>

class ISocketServer {
public:
    virtual ~ISocketServer() = default;
    
    virtual bool initialize() = 0;
    virtual void process() = 0;
    
    virtual void broadcastSensorData(const String& json) = 0;
    virtual void broadcastHeaterStatus(const String& json) = 0;
    virtual void broadcastSystemStatus(const String& json) = 0;
    
    virtual bool isWebSocketConnected() = 0;
};