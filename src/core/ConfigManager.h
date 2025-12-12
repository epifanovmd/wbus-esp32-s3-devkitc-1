#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include "./domain/Entities.h"
#include "./domain/Events.h"
#include "FileSystemManager.h"

enum class ConfigUpdateResult
{
    SUCCESS,
    SUCCESS_RESTART_REQUIRED,
    ERROR_SAVE_FAILED,
    ERROR_OTHER
};

class ConfigManager
{
private:
    EventBus &eventBus;
    FileSystemManager &fsManager;
    AppConfig config;
    String configPath = "/config.json";
    bool configLoaded = false;

    // Флаг для отслеживания необходимости перезагрузки
    bool restartRequired = false;

    // Конвертация строки в WifiMode
    NetworkConfig::WifiMode stringToWifiMode(const String &modeStr)
    {
        if (modeStr == "AP_STA")
            return NetworkConfig::WifiMode::AP_STA;
        return NetworkConfig::WifiMode::AP; // По умолчанию
    }

    // Конвертация WifiMode в строку
    String wifiModeToString(NetworkConfig::WifiMode &mode)
    {
        switch (mode)
        {
        case NetworkConfig::WifiMode::AP_STA:
            return "AP_STA";
        default:
            return "AP";
        }
    }

public:
    ConfigManager(EventBus &bus, FileSystemManager &fsMgr) : eventBus(bus), fsManager(fsMgr) {}

    const AppConfig &getConfig() const { return config; }
    bool isConfigLoaded() const { return configLoaded; }
    bool isRestartRequired() const { return restartRequired; }

    void initialize()
    {
        if (!loadConfig())
        {
            Serial.println("⚠️  Using default configuration");
        }
        // printConfig();
    }

    bool loadConfig()
    {
        // Инициализируем файловую систему, если нужно
        if (!fsManager.isInitialized() && !fsManager.begin())
        {
            Serial.println("⚠️ Cannot load config: filesystem not available");
            return false;
        }

        if (!fsManager.exists(configPath))
        {
            Serial.println("⚠️ Config file not found, creating with defaults");
            configLoaded = saveConfig();
            return configLoaded;
        }

        File file = fsManager.open(configPath, "r");
        if (!file)
        {
            Serial.println("❌ Failed to open config file");
            return false;
        }

        DynamicJsonDocument doc(2048);
        DeserializationError error = deserializeJson(doc, file);
        file.close();

        if (error)
        {
            Serial.println("❌ Failed to parse config file: " + String(error.c_str()));
            return false;
        }

        // Загружаем базовую информацию
        config.deviceId = doc["deviceId"] | "webasto-001";

        // Загружаем конфигурацию шины
        JsonObject bus = doc["bus"];
        config.bus.baudRate = bus["baudRate"] | 2400;
        config.bus.commandTimeout = bus["commandTimeout"] | 2000;
        config.bus.maxRetries = bus["maxRetries"] | 5;
        config.bus.queueInterval = bus["queueInterval"] | 150;
        config.bus.maxQueueSize = bus["maxQueueSize"] | 30;
        config.bus.maxPriorityQueueSize = bus["maxPriorityQueueSize"] | 10;
        config.bus.breakSignalDuration = bus["breakSignalDuration"] | 50;
        config.bus.keepAliveInterval = bus["keepAliveInterval"] | 15000;
        config.bus.nslpPin = bus["nslpPin"] | 7;
        config.bus.nwakePin = bus["nwakePin"] | 6;
        config.bus.rxdPullupPin = bus["rxdPullupPin"] | 8;
        config.bus.rxTjaPin = bus["rxTjaPin"] | 18;
        config.bus.txTjaPin = bus["txTjaPin"] | 17;
        config.bus.serialConfig = bus["serialConfig"] | "8E1";

        // Загружаем сетевую конфигурацию
        JsonObject network = doc["network"];

        // Режим WiFi (конвертируем из строки)
        String modeStr = network["mode"] | "AP_STA";
        config.network.mode = stringToWifiMode(modeStr);

        // STA настройки
        config.network.staSsid = network["staSsid"] | "";
        config.network.staPassword = network["staPassword"] | "";

        // AP настройки
        config.network.apSsid = network["apSsid"] | "Webasto-WiFi";
        config.network.apPassword = network["apPassword"] | "Epifan123";

        // Общие настройки
        config.network.hostname = network["hostname"] | "webasto-controller";
        config.network.port = network["port"] | 80;

        // Автопереподключение
        config.network.reconnectInterval = network["reconnectInterval"] | 10000;

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

        DynamicJsonDocument doc(2048);

        doc["deviceId"] = config.deviceId;

        JsonObject bus = doc.createNestedObject("bus");
        bus["baudRate"] = config.bus.baudRate;
        bus["commandTimeout"] = config.bus.commandTimeout;
        bus["maxRetries"] = config.bus.maxRetries;
        bus["queueInterval"] = config.bus.queueInterval;
        bus["maxQueueSize"] = config.bus.maxQueueSize;
        bus["maxPriorityQueueSize"] = config.bus.maxPriorityQueueSize;
        bus["breakSignalDuration"] = config.bus.breakSignalDuration;
        bus["keepAliveInterval"] = config.bus.keepAliveInterval;
        bus["nslpPin"] = config.bus.nslpPin;
        bus["nwakePin"] = config.bus.nwakePin;
        bus["rxdPullupPin"] = config.bus.rxdPullupPin;
        bus["rxTjaPin"] = config.bus.rxTjaPin;
        bus["txTjaPin"] = config.bus.txTjaPin;
        bus["serialConfig"] = config.bus.serialConfig;

        JsonObject network = doc.createNestedObject("network");

        // Сохраняем режим WiFi как строку
        network["mode"] = wifiModeToString(config.network.mode);

        // STA настройки
        network["staSsid"] = config.network.staSsid;
        network["staPassword"] = config.network.staPassword;

        // AP настройки
        network["apSsid"] = config.network.apSsid;
        network["apPassword"] = config.network.apPassword;

        // Общие настройки
        network["hostname"] = config.network.hostname;
        network["port"] = config.network.port;

        // Автопереподключение
        network["reconnectInterval"] = config.network.reconnectInterval;

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

    ConfigUpdateResult updateConfig(const JsonObject &newConfig)
    {
        bool needsRestart = false;

        // Сохраняем старые значения для сравнения
        uint32_t oldBaudRate = config.bus.baudRate;
        String oldSerialConfig = config.bus.serialConfig;
        uint8_t oldNslpPin = config.bus.nslpPin;
        uint8_t oldNwakePin = config.bus.nwakePin;
        uint8_t oldRxdPullupPin = config.bus.rxdPullupPin;
        uint8_t oldRxTjaPin = config.bus.rxTjaPin;
        uint8_t oldTxTjaPin = config.bus.txTjaPin;

        // Обновляем deviceId если есть
        if (newConfig.containsKey("deviceId"))
        {
            config.deviceId = newConfig["deviceId"].as<String>();
        }

        // Обновляем bus конфигурацию
        if (newConfig.containsKey("bus"))
        {
            JsonObject bus = newConfig["bus"];

            // Проверяем изменения, требующие перезагрузки
            if (bus.containsKey("baudRate") && bus["baudRate"] != oldBaudRate)
            {
                needsRestart = true;
                config.bus.baudRate = bus["baudRate"];
            }
            if (bus.containsKey("serialConfig"))
            {
                String newSerialConfig = bus["serialConfig"].as<String>();
                if (newSerialConfig != oldSerialConfig)
                {
                    needsRestart = true;
                    config.bus.serialConfig = newSerialConfig;
                }
            }
            if (bus.containsKey("nslpPin") && bus["nslpPin"] != oldNslpPin)
            {
                needsRestart = true;
                config.bus.nslpPin = bus["nslpPin"];
            }
            if (bus.containsKey("nwakePin") && bus["nwakePin"] != oldNwakePin)
            {
                needsRestart = true;
                config.bus.nwakePin = bus["nwakePin"];
            }
            if (bus.containsKey("rxdPullupPin") && bus["rxdPullupPin"] != oldRxdPullupPin)
            {
                needsRestart = true;
                config.bus.rxdPullupPin = bus["rxdPullupPin"];
            }
            if (bus.containsKey("rxTjaPin") && bus["rxTjaPin"] != oldRxTjaPin)
            {
                needsRestart = true;
                config.bus.rxTjaPin = bus["rxTjaPin"];
            }
            if (bus.containsKey("txTjaPin") && bus["txTjaPin"] != oldTxTjaPin)
            {
                needsRestart = true;
                config.bus.txTjaPin = bus["txTjaPin"];
            }

            // Параметры, не требующие перезагрузки
            if (bus.containsKey("commandTimeout"))
                config.bus.commandTimeout = bus["commandTimeout"];
            if (bus.containsKey("maxRetries"))
                config.bus.maxRetries = bus["maxRetries"];
            if (bus.containsKey("queueInterval"))
                config.bus.queueInterval = bus["queueInterval"];
            if (bus.containsKey("maxQueueSize"))
                config.bus.maxQueueSize = bus["maxQueueSize"];
            if (bus.containsKey("maxPriorityQueueSize"))
                config.bus.maxPriorityQueueSize = bus["maxPriorityQueueSize"];
            if (bus.containsKey("breakSignalDuration"))
                config.bus.breakSignalDuration = bus["breakSignalDuration"];
            if (bus.containsKey("keepAliveInterval"))
                config.bus.keepAliveInterval = bus["keepAliveInterval"];
        }

        // Обновляем network конфигурацию
        if (newConfig.containsKey("network"))
        {
            JsonObject network = newConfig["network"];

            // Смена порта требует перезагрузки сервера
            if (network.containsKey("port") && network["port"] != config.network.port)
            {
                needsRestart = true;
                config.network.port = network["port"];
            }

            // Смена hostname требует перезагрузки
            if (network.containsKey("hostname"))
            {
                String newHostname = network["hostname"].as<String>();
                if (newHostname != config.network.hostname)
                {
                    needsRestart = true;
                    config.network.hostname = newHostname;
                }
            }

            // Смена режима WiFi требует перезагрузки
            if (network.containsKey("mode"))
            {
                String modeStr = network["mode"].as<String>();
                NetworkConfig::WifiMode newMode = stringToWifiMode(modeStr);
                if (newMode != config.network.mode)
                {
                    needsRestart = true;
                    config.network.mode = newMode;
                }
            }

            // STA настройки (не требуют немедленной перезагрузки если не меняется режим)
            if (network.containsKey("staSsid"))
                config.network.staSsid = network["staSsid"].as<String>();
            if (network.containsKey("staPassword"))
                config.network.staPassword = network["staPassword"].as<String>();

            // AP настройки
            if (network.containsKey("apSsid"))
                config.network.apSsid = network["apSsid"].as<String>();
            if (network.containsKey("apPassword"))
                config.network.apPassword = network["apPassword"].as<String>();

            // Автопереподключение (не требует перезагрузки)
            if (network.containsKey("reconnectInterval"))
                config.network.reconnectInterval = network["reconnectInterval"];
        }

        // Сохраняем обновленную конфигурацию
        if (saveConfig())
        {
            eventBus.publish<AppConfigUpdateEvent>(EventType::APP_CONFIG_UPDATE, {config});
            if (needsRestart)
            {
                requestRestart();
                return ConfigUpdateResult::SUCCESS_RESTART_REQUIRED;
            }
            return ConfigUpdateResult::SUCCESS;
        }

        return ConfigUpdateResult::ERROR_SAVE_FAILED;
    }

    ConfigUpdateResult resetToDefaults()
    {
        Serial.println("🔄 Resetting configuration to defaults...");

        // Создаем новый конфиг с дефолтными значениями
        AppConfig defaultConfig;

        // Сравниваем с текущими значениями для определения необходимости перезагрузки
        bool needsRestart = false;

        if (config.bus.baudRate != defaultConfig.bus.baudRate ||
            config.bus.serialConfig != defaultConfig.bus.serialConfig ||
            config.bus.nslpPin != defaultConfig.bus.nslpPin ||
            config.bus.nwakePin != defaultConfig.bus.nwakePin ||
            config.bus.rxdPullupPin != defaultConfig.bus.rxdPullupPin ||
            config.bus.rxTjaPin != defaultConfig.bus.rxTjaPin ||
            config.bus.txTjaPin != defaultConfig.bus.txTjaPin ||
            config.network.port != defaultConfig.network.port ||
            config.network.hostname != defaultConfig.network.hostname ||
            config.network.mode != defaultConfig.network.mode)
        {
            needsRestart = true;
        }

        // Применяем дефолтные значения
        config = defaultConfig;

        // Сохраняем в файл
        if (saveConfig())
        {
            eventBus.publish<AppConfigUpdateEvent>(EventType::APP_CONFIG_UPDATE, {config});
            if (needsRestart)
            {
                requestRestart();
                return ConfigUpdateResult::SUCCESS_RESTART_REQUIRED;
            }
            Serial.println("✅ Configuration reset to defaults");
            return ConfigUpdateResult::SUCCESS;
        }

        Serial.println("❌ Failed to save default configuration");
        return ConfigUpdateResult::ERROR_SAVE_FAILED;
    }

    String getConfigJson()
    {
        DynamicJsonDocument doc(2048);

        doc["deviceId"] = config.deviceId;

        JsonObject bus = doc.createNestedObject("bus");
        bus["baudRate"] = config.bus.baudRate;
        bus["commandTimeout"] = config.bus.commandTimeout;
        bus["maxRetries"] = config.bus.maxRetries;
        bus["queueInterval"] = config.bus.queueInterval;
        bus["maxQueueSize"] = config.bus.maxQueueSize;
        bus["maxPriorityQueueSize"] = config.bus.maxPriorityQueueSize;
        bus["breakSignalDuration"] = config.bus.breakSignalDuration;
        bus["keepAliveInterval"] = config.bus.keepAliveInterval;
        bus["nslpPin"] = config.bus.nslpPin;
        bus["nwakePin"] = config.bus.nwakePin;
        bus["rxdPullupPin"] = config.bus.rxdPullupPin;
        bus["rxTjaPin"] = config.bus.rxTjaPin;
        bus["txTjaPin"] = config.bus.txTjaPin;
        bus["serialConfig"] = config.bus.serialConfig;

        JsonObject network = doc.createNestedObject("network");

        // Режим WiFi как строка
        network["mode"] = config.network.getModeString();

        // STA настройки (пароль скрываем)
        network["staSsid"] = config.network.staSsid;
        network["staPassword"] = config.network.staPassword;

        // AP настройки (пароль скрываем)
        network["apSsid"] = config.network.apSsid;
        network["apPassword"] = config.network.apPassword;

        // Общие настройки
        network["hostname"] = config.network.hostname;
        network["port"] = config.network.port;

        // Автопереподключение
        network["reconnectInterval"] = config.network.reconnectInterval;

        doc["restartRequired"] = restartRequired;

        String json;
        serializeJson(doc, json);
        return json;
    }

    void printConfig()
    {
        Serial.println("📋 Current Configuration:");
        Serial.println("  Device ID: " + config.deviceId);
        Serial.println("  Bus:");
        Serial.println("    Baud Rate: " + String(config.bus.baudRate));
        Serial.println("    Command Timeout: " + String(config.bus.commandTimeout));
        Serial.println("    Max Retries: " + String(config.bus.maxRetries));
        Serial.println("    Queue Interval: " + String(config.bus.queueInterval));
        Serial.println("    Max Queue Size: " + String(config.bus.maxQueueSize));
        Serial.println("    Max Priority Queue Size: " + String(config.bus.maxPriorityQueueSize));
        Serial.println("    Break Signal Duration: " + String(config.bus.breakSignalDuration));
        Serial.println("    Keep Alive Interval: " + String(config.bus.keepAliveInterval));
        Serial.println("    NSLP Pin: " + String(config.bus.nslpPin));
        Serial.println("    NWAKE Pin: " + String(config.bus.nwakePin));
        Serial.println("    RXD Pullup Pin: " + String(config.bus.rxdPullupPin));
        Serial.println("    RX TJA Pin: " + String(config.bus.rxTjaPin));
        Serial.println("    TX TJA Pin: " + String(config.bus.txTjaPin));
        Serial.println("    Serial Config: " + config.bus.serialConfig);

        Serial.println("  Network:");
        Serial.println("    Mode: " + wifiModeToString(config.network.mode));

        if (config.network.mode == NetworkConfig::WifiMode::AP_STA)
        {
            Serial.println("    STA SSID: " + config.network.staSsid);
            Serial.println("    STA Password: " + config.network.staPassword);
        }

        if (config.network.mode == NetworkConfig::WifiMode::AP || config.network.mode == NetworkConfig::WifiMode::AP_STA)
        {
            Serial.println("    AP SSID: " + config.network.apSsid);
            Serial.println("    AP Password: " + config.network.apPassword);
        }

        Serial.println("    Hostname: " + config.network.hostname);
        Serial.println("    Port: " + String(config.network.port));
        Serial.println("    Reconnect Interval: " + String(config.network.reconnectInterval) + "ms");
    }

private:
    // Запрос перезагрузки
    void requestRestart()
    {
        restartRequired = true;
        Serial.println("⚠️ Restart requested");
    }
};