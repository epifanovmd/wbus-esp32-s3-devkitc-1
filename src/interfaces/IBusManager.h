#pragma once
#include "../domain/Entities.h"

class IBusManager {
public:
    virtual ~IBusManager() = default;
    
    virtual bool initialize() = 0;
    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
    virtual ConnectionState getConnectionState() const = 0;
    
    virtual bool sendCommand(const String& command) = 0;
    virtual bool sendCommand(const String& command, std::function<void(String, String)> callback) = 0;
    
    virtual void wakeUp() = 0;
    virtual void sleep() = 0;
    virtual bool isAwake() const = 0;
    
    // Новые методы для BREAK сигнала
    virtual void sendBreakSignal(bool set) = 0;
    virtual int available() = 0;
    virtual uint8_t read() = 0;
    virtual void flush() = 0;
};