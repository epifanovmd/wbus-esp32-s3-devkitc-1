// src/core/ConfigManager.h
#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

struct BusConfig
{
    uint32_t baudRate = 2400;
    uint32_t commandTimeout = 2000;
    uint8_t maxRetries = 3;
    uint32_t queueInterval = 150;

    // Пины для управления TJA1020
    uint32_t NSLP_PIN = 7;               // Sleep control (active LOW)
    uint32_t NWAKE_PIN = 6;              // Wake-up input (active LOW)
    uint32_t RXD_PULLUP = 8;             // Пин для подтяжки RXD (open-drain)
    uint32_t SERIAL_CONFIG = SERIAL_8E1; // 8 бит, Even parity, 1 стоп-бит
    uint32_t RX_TJA_PIN = 18;
    uint32_t TX_TJA_PIN = 17;
};

struct NetworkConfig
{
    String ssid = "Webasto_WiFi";
    String password = "Epifan123";
    uint16_t webPort = 80;
};

struct AppConfig
{
    BusConfig bus;
    NetworkConfig network;
};

class ConfigManager
{
private:
    AppConfig config;
    String configPath = "/config.json";

public:
    static ConfigManager &getInstance()
    {
        static ConfigManager instance;
        return instance;
    }

    const AppConfig &getConfig() const { return config; }

    bool loadConfig()
    {
        // Проверяем, смонтирована ли файловая система
        bool fsMounted = LittleFS.begin(true);

        if (!fsMounted)
        {
            Serial.println("⚠️  LittleFS not mounted, using defaults");
            return false;
        }

        if (!LittleFS.exists(configPath))
        {
            Serial.println("⚠️  Config file not found, using defaults");
            // Пытаемся сохранить конфиг по умолчанию
            return saveConfig();
        }

        File file = LittleFS.open(configPath, "r");
        if (!file)
        {
            Serial.println("❌ Failed to open config file");
            LittleFS.end(); // Закрываем файловую систему
            return false;
        }

        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, file);
        file.close();

        if (error)
        {
            Serial.println("❌ Failed to parse config file: " + String(error.c_str()));
            LittleFS.end(); // Закрываем файловую систему
            return false;
        }

        // Load bus config
        config.bus.baudRate = doc["bus"]["baudRate"] | 2400;
        config.bus.commandTimeout = doc["bus"]["commandTimeout"] | 2000;
        config.bus.maxRetries = doc["bus"]["maxRetries"] | 3;
        config.bus.queueInterval = doc["bus"]["queueInterval"] | 150;

        // Load network config
        config.network.ssid = doc["network"]["ssid"] | "Webasto_WiFi";
        config.network.password = doc["network"]["password"] | "Epifan123";
        config.network.webPort = doc["network"]["webPort"] | 80;

        Serial.println("✅ Config loaded successfully");
        LittleFS.end(); // Закрываем файловую систему
        return true;
    }

    bool saveConfig()
    {
        // Проверяем, смонтирована ли файловая система
        bool fsMounted = LittleFS.begin(true);

        if (!fsMounted)
        {
            Serial.println("❌ LittleFS not mounted, cannot save config");
            return false;
        }

        DynamicJsonDocument doc(1024);

        doc["bus"]["baudRate"] = config.bus.baudRate;
        doc["bus"]["commandTimeout"] = config.bus.commandTimeout;
        doc["bus"]["maxRetries"] = config.bus.maxRetries;
        doc["bus"]["queueInterval"] = config.bus.queueInterval;

        doc["network"]["ssid"] = config.network.ssid;
        doc["network"]["password"] = config.network.password;
        doc["network"]["webPort"] = config.network.webPort;

        File file = LittleFS.open(configPath, "w");
        if (!file)
        {
            Serial.println("❌ Failed to create config file");
            LittleFS.end(); // Закрываем файловую систему
            return false;
        }

        serializeJson(doc, file);
        file.close();

        Serial.println("✅ Config saved successfully");
        LittleFS.end(); // Закрываем файловую систему
        return true;
    }

    void printConfig()
    {
        Serial.println("📋 Current Configuration:");
        Serial.println("  Bus:");
        Serial.println("    Baud Rate: " + String(config.bus.baudRate));
        Serial.println("    Command Timeout: " + String(config.bus.commandTimeout));
        Serial.println("    Max Retries: " + String(config.bus.maxRetries));
        Serial.println("    Queue Interval: " + String(config.bus.queueInterval));
        Serial.println("  Network:");
        Serial.println("    SSID: " + config.network.ssid);
        Serial.println("    Password: " + config.network.password);
        Serial.println("    Web Port: " + String(config.network.webPort));
    }
};