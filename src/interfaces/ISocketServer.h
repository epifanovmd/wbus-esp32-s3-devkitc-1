// src/interfaces/ISocketServer.h
#pragma once
#include <Arduino.h>
#include <functional>

class ISocketServer {
public:
    virtual ~ISocketServer() = default;
    
    virtual bool initialize() = 0;
    virtual void process() = 0;
    
    virtual void broadcastJson(EventType eventType, const String &json) = 0;
    
    virtual bool isWebSocketConnected() = 0;
};