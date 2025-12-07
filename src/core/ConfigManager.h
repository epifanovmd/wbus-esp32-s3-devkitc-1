#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include "FileSystemManager.h"

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
    uint16_t port = 80;
};

struct AppConfig
{
    BusConfig bus;
    NetworkConfig network;
};

class ConfigManager
{
private:
    FileSystemManager &fsManager;
    AppConfig config;
    String configPath = "/config.json";
    bool configLoaded = false;

public:
    ConfigManager(FileSystemManager &fsMgr) : fsManager(fsMgr) {}

    const AppConfig &getConfig() const { return config; }
    bool isConfigLoaded() const { return configLoaded; }

    bool loadConfig()
    {
        // Инициализируем файловую систему, если нужно
        if (!fsManager.isInitialized() && !fsManager.begin())
        {
            Serial.println("⚠️  Cannot load config: filesystem not available");
            return false;
        }

        if (!fsManager.exists(configPath))
        {
            Serial.println("⚠️  Config file not found, using defaults");
            // Пытаемся сохранить конфиг по умолчанию
            configLoaded = saveConfig();
            return configLoaded;
        }

        File file = fsManager.open(configPath, "r");
        if (!file)
        {
            Serial.println("❌ Failed to open config file");
            return false;
        }

        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, file);
        file.close();

        if (error)
        {
            Serial.println("❌ Failed to parse config file: " + String(error.c_str()));
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
        config.network.port = doc["network"]["port"] | 80;

        configLoaded = true;
        Serial.println("✅ Config loaded successfully");
        return true;
    }

    bool saveConfig()
    {
        if (!fsManager.isInitialized() && !fsManager.begin())
        {
            Serial.println("❌ Cannot save config: filesystem not available");
            return false;
        }

        DynamicJsonDocument doc(1024);

        doc["bus"]["baudRate"] = config.bus.baudRate;
        doc["bus"]["commandTimeout"] = config.bus.commandTimeout;
        doc["bus"]["maxRetries"] = config.bus.maxRetries;
        doc["bus"]["queueInterval"] = config.bus.queueInterval;

        doc["network"]["ssid"] = config.network.ssid;
        doc["network"]["password"] = config.network.password;
        doc["network"]["port"] = config.network.port;

        File file = fsManager.open(configPath, "w");
        if (!file)
        {
            Serial.println("❌ Failed to create config file");
            return false;
        }

        serializeJson(doc, file);
        file.close();

        configLoaded = true;
        Serial.println("✅ Config saved successfully");
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
        Serial.println("    NSLP Pin: " + String(config.bus.NSLP_PIN));
        Serial.println("    NWAKE Pin: " + String(config.bus.NWAKE_PIN));
        Serial.println("  Network:");
        Serial.println("    SSID: " + config.network.ssid);
        Serial.println("    Password: " + config.network.password);
        Serial.println("    Port: " + String(config.network.port));
    }

    // Метод для обновления конфигурации
    bool updateConfig(const AppConfig &newConfig)
    {
        config = newConfig;
        return saveConfig();
    }
};