#pragma once
#include "../../interfaces/IBusManager.h"
#include "../../core/EventBus.h"
#include "../../core/ConfigManager.h"
#include "./domain/Entities.h"
#include "../../domain/Events.h"
#include "../../common/Constants.h"

class TJA1020Driver : public IBusManager {
private:
    HardwareSerial& serial;
    EventBus& eventBus;
    const BusConfig& config;
    
    ConnectionState connectionState = ConnectionState::DISCONNECTED;
    bool isAwakeFlag = false;

public:
    TJA1020Driver(HardwareSerial& serialRef, EventBus& bus) 
        : serial(serialRef), eventBus(bus), config(ConfigManager::getInstance().getConfig().bus) {}
    
    bool initialize() override {
        // Инициализация пинов управления TJA1020
        pinMode(NSLP_PIN, OUTPUT);
        pinMode(NWAKE_PIN, OUTPUT);
        pinMode(RXD_PULLUP, OUTPUT);

        // Подтяжка RXD к 3.3V
        digitalWrite(RXD_PULLUP, HIGH);

        // Изначально спящий режим
        digitalWrite(NSLP_PIN, LOW);
        digitalWrite(NWAKE_PIN, HIGH);
        
        Serial.println("✅ TJA1020 Driver initialized");
        return true;
    }
    
    bool connect() override {
        if (connectionState == ConnectionState::CONNECTING) {
            Serial.println("⚠️  Already connecting...");
            return false;
        }
        
        setConnectionState(ConnectionState::CONNECTING);
        wakeUp();
        
        // Инициализация UART из оригинального кода
        serial.begin(2400, SERIAL_8E1, 18, 17);
        
        setConnectionState(ConnectionState::CONNECTED);
        Serial.println("✅ TJA1020 connected");
        return true;
    }
    
    void disconnect() override {
        sleep();
        setConnectionState(ConnectionState::DISCONNECTED);
        Serial.println("🔌 TJA1020 disconnected");
    }
    
    bool isConnected() const override {
        return connectionState == ConnectionState::CONNECTED && isAwakeFlag;
    }
    
    ConnectionState getConnectionState() const override {
        return connectionState;
    }
    
    void wakeUp() override {
        digitalWrite(NSLP_PIN, HIGH);
        delay(10);

        digitalWrite(NWAKE_PIN, LOW);
        delay(2);
        digitalWrite(NWAKE_PIN, HIGH);

        delay(50);
        isAwakeFlag = true;
        
        Serial.println("🔔 TJA1020 awakened");
    }

    void sendBreak() override {
        sendBreakSignal(true);
        delay(50);

        sendBreakSignal(false);
        delay(50);
    }
    
    void sleep() override {
        digitalWrite(TX_TJA_PIN, HIGH);
        delay(10);
        
        serial.end();
        digitalWrite(NSLP_PIN, LOW);
        digitalWrite(NWAKE_PIN, HIGH);
        
        delay(10);
        isAwakeFlag = false;
        
        Serial.println("💤 TJA1020 sleeping");
    }
    
    bool isAwake() const override {
        return isAwakeFlag;
    }
    
    bool sendCommand(uint8_t *data, size_t length) override {
        if (!isAwakeFlag) {
            Serial.println("❌ TJA1020 is sleeping");
            return false;
        }
        
        for (int i = 0; i < length; i++) {
            serial.write(data[i]);
        }
        
        serial.flush();
        
        return true;
    }
    
    // =========================================================================
    // РЕАЛИЗАЦИЯ НОВЫХ МЕТОДОВ ДЛЯ BREAK СИГНАЛА
    // =========================================================================
    
    void sendBreakSignal(bool set) override {
        if (set) {
            // BREAK set - удерживаем линию в LOW (как в оригинале)
            serial.write(0x00); // Отправляем нулевой байт для BREAK
        } else {
            // BREAK reset - отпускаем линию
            serial.flush(); // Очищаем буфер
        }
    }
    
    // Методы для работы с serial (нужны для WBusReceiver)
    int available() override {
        return serial.available();
    }
    
    uint8_t read() override {
        return serial.read();
    }
    
    void flush() override {
        serial.flush();
    }

private:
    void setConnectionState(ConnectionState newState) {
        if (connectionState != newState) {
            ConnectionState oldState = connectionState;
            connectionState = newState;

            // eventBus.publish<ConnectionStateChangedEvent>(EventType::CONNECTION_STATE_CHANGED, {oldState, newState});
        }
    }
};