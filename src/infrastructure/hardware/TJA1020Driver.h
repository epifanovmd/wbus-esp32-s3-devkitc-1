#pragma once
#include "../../interfaces/IBusManager.h"
#include "../../core/EventBus.h"
#include "../../core/ConfigManager.h"
#include "./domain/Entities.h"
#include "../../domain/Events.h"
#include "../../common/Constants.h"

class TJA1020Driver : public IBusManager
{
private:
    HardwareSerial &serial;
    EventBus &eventBus;
    ConfigManager &configManager;
    const BusConfig &config;

    ConnectionState connectionState = ConnectionState::DISCONNECTED;

public:
    TJA1020Driver(ConfigManager &configMmngr, HardwareSerial &serialRef, EventBus &bus)
        : serial(serialRef), eventBus(bus), configManager(configMmngr), config(configManager.getConfig().bus) {}

    bool initialize() override
    {
        // Инициализация пинов управления TJA1020
        pinMode(config.nslpPin, OUTPUT);
        pinMode(config.nwakePin, OUTPUT);
        pinMode(config.rxdPullupPin, OUTPUT);

        // Подтяжка RXD к 3.3V
        digitalWrite(config.rxdPullupPin, HIGH);

        // Изначально спящий режим
        digitalWrite(config.nslpPin, LOW);
        digitalWrite(config.nwakePin, HIGH);

        // Serial.println("✅ TJA1020 Driver initialized");
        // Serial.println("  NSLP Pin: " + String(config.nslpPin));
        // Serial.println("  NWAKE Pin: " + String(config.nwakePin));
        // Serial.println("  RXD Pullup Pin: " + String(config.rxdPullupPin));
        // Serial.println("  TX Pin: " + String(config.txTjaPin));
        // Serial.println("  RX Pin: " + String(config.rxTjaPin));
        return true;
    }

    void connect() override
    {
        if (connectionState == ConnectionState::CONNECTING)
        {
            Serial.println("⚠️ Already connecting...");
            return;
        }

        wakeUp();
    }

    void disconnect() override
    {
        sleep();
    }

    bool isConnected() const override
    {
        return connectionState == ConnectionState::CONNECTED;
    }

    ConnectionState getConnectionState() const override
    {
        return connectionState;
    }

    void wakeUp() override
    {
        setConnectionState(ConnectionState::CONNECTING);
        digitalWrite(config.nslpPin, HIGH);
        delay(10);

        digitalWrite(config.nwakePin, LOW);
        delay(2);
        digitalWrite(config.nwakePin, HIGH);

        delay(config.breakSignalDuration);

        setConnectionState(ConnectionState::CONNECTED);

        // Инициализация UART с параметрами из конфига
        serial.begin(config.baudRate, config.getSerialConfig(), config.rxTjaPin, config.txTjaPin);

        setConnectionState(ConnectionState::CONNECTED);
        Serial.println("✅ TJA1020 connected");
    }

    void sendBreak() override
    {
        sendBreakSignal(true);
        delay(config.breakSignalDuration);

        sendBreakSignal(false);
        delay(config.breakSignalDuration);
    }

    void sleep() override
    {
        digitalWrite(config.txTjaPin, HIGH);
        delay(10);

        serial.end();
        digitalWrite(config.nslpPin, LOW);
        digitalWrite(config.nwakePin, HIGH);

        delay(10);

        setConnectionState(ConnectionState::DISCONNECTED);

        Serial.println("💤 TJA1020 sleeping");
    }

    bool sendCommand(uint8_t *data, size_t length) override
    {
        if (!isConnected())
        {
            Serial.println("❌ TJA1020 is sleeping");
            return false;
        }

        for (int i = 0; i < length; i++)
        {
            serial.write(data[i]);
        }

        serial.flush();

        return true;
    }

    void sendBreakSignal(bool set) override
    {
        if (set)
        {
            // BREAK set - удерживаем линию в LOW
            serial.write(0x00); // Отправляем нулевой байт для BREAK
        }
        else
        {
            // BREAK reset - отпускаем линию
            serial.flush(); // Очищаем буфер
        }
    }

    // Методы для работы с serial
    int available() override
    {
        return serial.available();
    }

    uint8_t read() override
    {
        return serial.read();
    }

    void flush() override
    {
        serial.flush();
    }

private:
    void setConnectionState(ConnectionState newState)
    {
        if (connectionState != newState)
        {
            ConnectionState oldState = connectionState;
            connectionState = newState;

            // eventBus.publish<ConnectionStateChangedEvent>(EventType::CONNECTION_STATE_CHANGED, {oldState, newState});
        }
    }
};