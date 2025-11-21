// src/core/ConfigManager.h
#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <LittleFS.h>

struct BusConfig {
    uint32_t baudRate = 2400;
    uint32_t commandTimeout = 2000;
    uint8_t maxRetries = 3;
    uint32_t queueInterval = 150;
};

struct NetworkConfig {
    String ssid = "Webasto_WiFi";
    String password = "Epifan123";
    uint16_t webPort = 80;
    uint16_t wsPort = 81;
};

struct HeaterConfig {
    uint8_t defaultRuntime = 60;
    float overheatThreshold = 90.0;
    float lowVoltageThreshold = 11.0;
};

struct AppConfig {
    BusConfig bus;
    NetworkConfig network;
    HeaterConfig heater;
};

class ConfigManager {
private:
    AppConfig config;
    String configPath = "/config.json";

public:
    static ConfigManager& getInstance() {
        static ConfigManager instance;
        return instance;
    }
    
    const AppConfig& getConfig() const { return config; }
    
    bool loadConfig() {
        if (!LittleFS.exists(configPath)) {
            Serial.println("⚠️  Config file not found, using defaults");
            return saveConfig();
        }
        
        File file = LittleFS.open(configPath, "r");
        if (!file) {
            Serial.println("❌ Failed to open config file");
            return false;
        }
        
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, file);
        file.close();
        
        if (error) {
            Serial.println("❌ Failed to parse config file: " + String(error.c_str()));
            return false;
        }
        
        // Load bus config - ИСПРАВЛЕНО: используем правильные имена полей
        config.bus.baudRate = doc["bus"]["baudRate"] | 2400;
        config.bus.commandTimeout = doc["bus"]["commandTimeout"] | 2000;
        config.bus.maxRetries = doc["bus"]["maxRetries"] | 3;
        config.bus.queueInterval = doc["bus"]["queueInterval"] | 150;
        
        // Load network config
        config.network.ssid = doc["network"]["ssid"] | "Webasto_WiFi";
        config.network.password = doc["network"]["password"] | "Epifan123";
        config.network.webPort = doc["network"]["webPort"] | 80;
        config.network.wsPort = doc["network"]["wsPort"] | 81;
        
        // Load heater config
        config.heater.defaultRuntime = doc["heater"]["defaultRuntime"] | 60;
        config.heater.overheatThreshold = doc["heater"]["overheatThreshold"] | 90.0;
        config.heater.lowVoltageThreshold = doc["heater"]["lowVoltageThreshold"] | 11.0;
        
        Serial.println("✅ Config loaded successfully");
        return true;
    }
    
    bool saveConfig() {
        DynamicJsonDocument doc(1024);
        
        doc["bus"]["baudRate"] = config.bus.baudRate;
        doc["bus"]["commandTimeout"] = config.bus.commandTimeout;
        doc["bus"]["maxRetries"] = config.bus.maxRetries;
        doc["bus"]["queueInterval"] = config.bus.queueInterval;
        
        doc["network"]["ssid"] = config.network.ssid;
        doc["network"]["password"] = config.network.password;
        doc["network"]["webPort"] = config.network.webPort;
        doc["network"]["wsPort"] = config.network.wsPort;
        
        doc["heater"]["defaultRuntime"] = config.heater.defaultRuntime;
        doc["heater"]["overheatThreshold"] = config.heater.overheatThreshold;
        doc["heater"]["lowVoltageThreshold"] = config.heater.lowVoltageThreshold;
        
        File file = LittleFS.open(configPath, "w");
        if (!file) {
            Serial.println("❌ Failed to create config file");
            return false;
        }
        
        serializeJson(doc, file);
        file.close();
        
        Serial.println("✅ Config saved successfully");
        return true;
    }
    
    void printConfig() {
        Serial.println("📋 Current Configuration:");
        Serial.println("  Bus:");
        Serial.println("    Baud Rate: " + String(config.bus.baudRate));
        Serial.println("    Command Timeout: " + String(config.bus.commandTimeout));
        Serial.println("    Max Retries: " + String(config.bus.maxRetries));
        Serial.println("    Queue Interval: " + String(config.bus.queueInterval));
        Serial.println("  Network:");
        Serial.println("    SSID: " + config.network.ssid);
        Serial.println("    Web Port: " + String(config.network.webPort));
        Serial.println("    WS Port: " + String(config.network.wsPort));
        Serial.println("  Heater:");
        Serial.println("    Default Runtime: " + String(config.heater.defaultRuntime));
        Serial.println("    Overheat Threshold: " + String(config.heater.overheatThreshold));
        Serial.println("    Low Voltage Threshold: " + String(config.heater.lowVoltageThreshold));
    }
};