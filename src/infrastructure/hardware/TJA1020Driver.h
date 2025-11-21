#pragma once
#include "../../interfaces/IBusManager.h"
#include "../../core/EventBus.h"
#include "../../core/ConfigManager.h"
#include "../protocol/WBusProtocol.h"
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
        // Инициализация пинов из оригинального tja1020.cpp
        pinMode(NSLP_PIN, OUTPUT);
        pinMode(NWAKE_PIN, OUTPUT);
        pinMode(RXD_PULLUP, OUTPUT);
        
        digitalWrite(RXD_PULLUP, HIGH);
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
        serial.begin(config.baudRate, SERIAL_8E1, RX_TJA_PIN, TX_TJA_PIN);
        
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
        // Реализация из оригинального wakeUpTJA1020()
        digitalWrite(NSLP_PIN, HIGH);
        delay(10);
        
        digitalWrite(NWAKE_PIN, LOW);
        delay(2);
        digitalWrite(NWAKE_PIN, HIGH);
        
        delay(50);
        isAwakeFlag = true;
        
        Serial.println("🔔 TJA1020 awakened");
    }
    
    void sleep() override {
        // Реализация из оригинального sleepTJA1020()
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
    
    bool sendCommand(const String& command) override {
        if (!isAwakeFlag) {
            Serial.println("❌ TJA1020 is sleeping");
            return false;
        }
        
        WBusPacket packet = WBusProtocol::parseHexStringToPacket(command);
        if (!WBusProtocol::validateWbusPacket(packet)) {
            Serial.println("❌ Invalid W-Bus packet");
            return false;
        }
        
        for (int i = 0; i < packet.byteCount; i++) {
            serial.write(packet.data[i]);
        }
        
        serial.flush(); // Ensure data is sent
        
        eventBus.publish<CommandSentEvent>(
            EventType::COMMAND_SENT,
            {command, "W-Bus command sent"}
        );
        
        return true;
    }
    
    bool sendCommand(const String& command, std::function<void(String, String)> callback) override {
        bool result = sendCommand(command);
        if (callback) {
            callback(command, result ? "sent" : "failed");
        }
        return result;
    }
    
    // =========================================================================
    // РЕАЛИЗАЦИЯ НОВЫХ МЕТОДОВ ДЛЯ BREAK СИГНАЛА
    // =========================================================================
    
    void sendBreakSignal(bool set) override {
        if (set) {
            // BREAK set - удерживаем линию в LOW (как в оригинале)
            serial.write(0x00); // Отправляем нулевой байт для BREAK
            Serial.println("🔧 BREAK signal SET");
        } else {
            // BREAK reset - отпускаем линию
            serial.flush(); // Очищаем буфер
            Serial.println("🔧 BREAK signal RESET");
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
            eventBus.publish<ConnectionStateChangedEvent>(
                EventType::CONNECTION_STATE_CHANGED,
                {oldState, newState, "Hardware state change"}
            );
        }
    }
};