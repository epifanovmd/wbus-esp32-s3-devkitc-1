#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include "FileSystemManager.h"

struct BusConfig
{
    uint32_t baudRate = 2400;
    uint32_t commandTimeout = 2000;
    uint8_t maxRetries = 5;
    uint32_t queueInterval = 150;
    uint32_t maxQueueSize = 30;
    uint32_t maxPriorityQueueSize = 10;
    uint32_t breakSignalDuration = 50;
    uint32_t keepAliveInterval = 15000;

    // Пины для управления TJA1020
    uint8_t nslpPin = 7;
    uint8_t nwakePin = 6;
    uint8_t rxdPullupPin = 8;
    uint8_t rxTjaPin = 18;
    uint8_t txTjaPin = 17;

    // Серийная конфигурация (хранится как строка, преобразуется при использовании)
    String serialConfig = "8E1";

    // Вспомогательный метод для получения конфигурации UART
    uint32_t getSerialConfig() const
    {
        if (serialConfig == "8N1")
            return SERIAL_8N1;
        if (serialConfig == "8E1")
            return SERIAL_8E1;
        if (serialConfig == "8O1")
            return SERIAL_8O1;
        if (serialConfig == "7E1")
            return SERIAL_7E1;
        if (serialConfig == "7O1")
            return SERIAL_7O1;
        return SERIAL_8E1; // по умолчанию
    }
};

struct NetworkConfig
{
    String ssid = "Webasto_WiFi";
    String password = "Epifan123";
    uint16_t port = 80;
    String otaUsername = "admin";
    String otaPassword = "Epifan123";
};

struct AppConfig
{
    uint8_t configVersion = 2;
    String deviceId = "webasto-001";
    BusConfig bus;
    NetworkConfig network;
};

enum class ConfigUpdateResult
{
    SUCCESS,
    SUCCESS_RESTART_REQUIRED,
    ERROR_INVALID_VERSION,
    ERROR_SAVE_FAILED,
    ERROR_OTHER
};

class ConfigManager
{
private:
    FileSystemManager &fsManager;
    AppConfig config;
    String configPath = "/config.json";
    bool configLoaded = false;

    // Флаг для отслеживания необходимости перезагрузки
    bool restartRequired = false;

    // Время, когда была запрошена перезагрузка
    unsigned long restartRequestTime = 0;

    // Задержка перед перезагрузкой (мс)
    static const unsigned long RESTART_DELAY = 2000;

public:
    ConfigManager(FileSystemManager &fsMgr) : fsManager(fsMgr) {}

    const AppConfig &getConfig() const { return config; }
    bool isConfigLoaded() const { return configLoaded; }
    bool isRestartRequired() const { return restartRequired; }

    void checkRestart()
    {
        if (restartRequired && millis() - restartRequestTime >= RESTART_DELAY)
        {
            performRestart();
        }
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
        config.configVersion = doc["configVersion"] | 2;
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
        config.network.ssid = network["ssid"] | "Webasto_WiFi";
        config.network.password = network["password"] | "Epifan123";
        config.network.port = network["port"] | 80;
        config.network.otaUsername = network["otaUsername"] | "admin";
        config.network.otaPassword = network["otaPassword"] | "Epifan123";

        configLoaded = true;
        Serial.println("✅ Config loaded successfully (version: " + String(config.configVersion) + ")");
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

        doc["configVersion"] = config.configVersion;
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
        network["ssid"] = config.network.ssid;
        network["password"] = config.network.password;
        network["port"] = config.network.port;
        network["otaUsername"] = config.network.otaUsername;
        network["otaPassword"] = config.network.otaPassword;

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
        // Проверяем версию конфига
        if (!newConfig.containsKey("configVersion") || newConfig["configVersion"] < 2)
        {
            return ConfigUpdateResult::ERROR_INVALID_VERSION;
        }

        bool needsRestart = false;

        // Сохраняем старые значения для сравнения
        uint32_t oldBaudRate = config.bus.baudRate;
        String oldSerialConfig = config.bus.serialConfig;
        uint8_t oldNslpPin = config.bus.nslpPin;
        uint8_t oldNwakePin = config.bus.nwakePin;
        uint8_t oldRxdPullupPin = config.bus.rxdPullupPin;
        uint8_t oldRxTjaPin = config.bus.rxTjaPin;
        uint8_t oldTxTjaPin = config.bus.txTjaPin;

        uint16_t oldPort = config.network.port;

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
            if (network.containsKey("port") && network["port"] != oldPort)
            {
                needsRestart = true;
                config.network.port = network["port"];
            }

            // Остальные сетевые параметры не требуют перезагрузки
            if (network.containsKey("ssid"))
                config.network.ssid = network["ssid"].as<String>();
            if (network.containsKey("password"))
                config.network.password = network["password"].as<String>();
            if (network.containsKey("otaUsername"))
                config.network.otaUsername = network["otaUsername"].as<String>();
            if (network.containsKey("otaPassword"))
                config.network.otaPassword = network["otaPassword"].as<String>();
        }

        // Сохраняем обновленную конфигурацию
        if (saveConfig())
        {
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
            config.network.port != defaultConfig.network.port)
        {
            needsRestart = true;
        }

        // Применяем дефолтные значения
        config = defaultConfig;

        // Сохраняем в файл
        if (saveConfig())
        {
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

    String getConfigJson() const
    {
        DynamicJsonDocument doc(2048);

        doc["configVersion"] = config.configVersion;
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
        network["ssid"] = config.network.ssid;
        network["password"] = config.network.password;
        network["port"] = config.network.port;
        network["otaUsername"] = config.network.otaUsername;
        network["otaPassword"] = config.network.otaPassword;

        String json;
        serializeJson(doc, json);
        return json;
    }

    void printConfig()
    {
        Serial.println("📋 Current Configuration:");
        Serial.println("  Version: " + String(config.configVersion));
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
        Serial.println("    SSID: " + config.network.ssid);
        Serial.println("    Password: " + config.network.password);
        Serial.println("    Port: " + String(config.network.port));
        Serial.println("    OTA Username: " + config.network.otaUsername);
        Serial.println("    OTA Password: " + config.network.otaPassword);
    }

private:
    // Запрос перезагрузки
    void requestRestart()
    {
        restartRequired = true;
        restartRequestTime = millis();
        Serial.println("⚠️ Restart requested. Controller will reboot in " +
                       String(RESTART_DELAY / 1000) + " seconds...");
    }

    // Выполнение перезагрузки
    void performRestart()
    {
        Serial.println("🔄 Performing controller restart...");
        delay(100); // Даем время на запись в Serial
        ESP.restart();
    }
};