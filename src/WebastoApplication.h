#pragma once
#include <WiFi.h>
#include "core/EventBus.h"
#include "core/ConfigManager.h"
#include "core/FileSystemManager.h"
#include "infrastructure/hardware/TJA1020Driver.h"
#include "infrastructure/network/AsyncWebServer.h"
#include "application/CommandManager.h"
#include "application/SensorManager.h"
#include "application/HeaterController.h"
#include "application/DeviceInfoManager.h"
#include "application/ErrorsManager.h"
#include "application/CommandReceiver.h"
#include "application/SnifferManager.h"
#include "common/Utils.h"
#include "common/Constants.h"
#include "infrastructure/protocol/WBusCommandBuilder.h"

class WebastoApplication
{
private:
    EventBus &eventBus;
    ConfigManager configManager;
    FileSystemManager fileSystemManager;

    // Аппаратный слой
    TJA1020Driver busDriver;
    HardwareSerial KLineSerial;

    // Приемник W-Bus пакетов
    CommandReceiver commandReceiver;
    // Управление командами
    CommandManager commandManager;

    // Бизнес-логика
    DeviceInfoManager deviceInfoManager;
    SensorManager sensorManager;
    ErrorsManager errorsManager;
    HeaterController heaterController;
    SnifferManager snifferManager;

    AsyncApiServer asyncWebServer;

    Timer keepAliveTimer;

    // Состояние приложения
    bool initialized = false;

    // Кнопка управления (пин 0)
    static const int BUTTON_PIN = 0;
    bool lastButtonState = true;

public:
    WebastoApplication() : eventBus(EventBus::getInstance()),
                           fileSystemManager(),
                           configManager(eventBus, fileSystemManager), KLineSerial(1),
                           busDriver(configManager, KLineSerial, eventBus),
                           commandReceiver(KLineSerial, eventBus),
                           commandManager(configManager, eventBus, busDriver, commandReceiver),
                           deviceInfoManager(eventBus, commandManager),
                           sensorManager(eventBus, commandManager),
                           errorsManager(eventBus, commandManager),
                           heaterController(eventBus, commandManager, busDriver, deviceInfoManager, sensorManager, errorsManager),
                           snifferManager(eventBus, deviceInfoManager, sensorManager, errorsManager, heaterController),
                           asyncWebServer(fileSystemManager, configManager, deviceInfoManager, sensorManager, errorsManager, heaterController),
                           keepAliveTimer(15000)
    {
    }

    void initialize()
    {
        Serial.begin(115200);
        Serial.println();
        Serial.println("🚗 Webasto W-Bus Controller");
        Serial.println("Device ID: " + configManager.getConfig().deviceId);
        Serial.println("===============================================");

        // Инициализация WiFi
        setupWiFi();

        configManager.initialize();
        busDriver.initialize();

        commandManager.initialize();
        heaterController.initialize();

        setupEventHandlers();

        busDriver.connect();
        asyncWebServer.initialize();

        initialized = true;

        Serial.println("✅ Webasto Application initialized successfully");
    }

    void process()
    {
        if (!initialized)
            return;

        commandReceiver.process();
        commandManager.process();

        if (keepAliveTimer.isReady())
        {
            processKeepAlive();
        }

        handleSerialCommands();
        handleButton();

        asyncWebServer.process();

        delay(1);
    }

private:
    void setupWiFi()
    {
        auto &netConfig = configManager.getConfig().network;

        Serial.println();
        Serial.println("📡 Starting Access Point...");

        // Простая и надежная версия
        WiFi.mode(WIFI_AP);

        WiFi.onEvent([](WiFiEvent_t event, arduino_event_info_t info)
                     {
        switch(event) {
            case ARDUINO_EVENT_WIFI_AP_START:
                Serial.println("✅ AP started");
                break;
            case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
                Serial.printf("📱 Client connected: MAC=%02x:%02x:%02x:%02x:%02x:%02x, AID=%d\n",
                    info.wifi_ap_staconnected.mac[0], info.wifi_ap_staconnected.mac[1],
                    info.wifi_ap_staconnected.mac[2], info.wifi_ap_staconnected.mac[3],
                    info.wifi_ap_staconnected.mac[4], info.wifi_ap_staconnected.mac[5],
                    info.wifi_ap_staconnected.aid);
                break;
            case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
                Serial.printf("📱 Client disconnected: MAC=%02x:%02x:%02x:%02x:%02x:%02x, AID=%d\n",
                    info.wifi_ap_staconnected.mac[0], info.wifi_ap_staconnected.mac[1],
                    info.wifi_ap_staconnected.mac[2], info.wifi_ap_staconnected.mac[3],
                    info.wifi_ap_staconnected.mac[4], info.wifi_ap_staconnected.mac[5],
                    info.wifi_ap_staconnected.aid);
                break;
            case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
                Serial.println("📱 Client IP assigned");
                break;
        } });

        // Базовые настройки
        WiFi.softAPConfig(
            IPAddress(192, 168, 4, 1),
            IPAddress(192, 168, 4, 1),
            IPAddress(255, 255, 255, 0));

        // Запуск AP
        if (WiFi.softAP(netConfig.ssid.c_str(), netConfig.password.c_str()))
        {
            Serial.println("\n✅ Access Point started successfully");
            Serial.println("  SSID: " + netConfig.ssid);
            Serial.println("  IP Address: " + WiFi.softAPIP().toString());
            Serial.println("  MAC Address: " + WiFi.softAPmacAddress());
            Serial.println("  Channel: " + String(WiFi.channel()));

            // Дополнительные оптимизации
            optimizeWiFi();
        }
        else
        {
            Serial.println("❌ Failed to start Access Point");
            delay(1000);
            ESP.restart();
        }
    }

    void optimizeWiFi()
    {
        // Базовые оптимизации совместимые со всеми версиями

        // 1. Отключаем режим сна WiFi
        WiFi.setSleep(false);

// 2. Устанавливаем максимальную мощность передачи
#if defined(ESP_ARDUINO_VERSION_MAJOR) && ESP_ARDUINO_VERSION_MAJOR >= 2
        WiFi.setTxPower(WIFI_POWER_19_5dBm);
#elif defined(ARDUINO_ESP32_RELEASE_1_0_x)
        WiFi.setTxPower(78); // 19.5dBm
#endif

        // 3. Устанавливаем статический канал (по умолчанию 6)
        // ESP автоматически выбирает канал, но можно принудительно:
        // WiFi.softAP("SSID", "PASS", 6); // в setupWiFi()

        // 4. Отключаем автоматическое переподключение (для AP не нужно)

        Serial.println("🔧 WiFi optimized for stability");
    }

    void setupEventHandlers()
    {
        HeaterStatus status;

        eventBus.subscribe(EventType::TX_RECEIVED,
                           [](const Event &event)
                           {
                               // Serial.println();
                               // Serial.print("📤 TX: " + event.source);
                           });

        eventBus.subscribe(EventType::RX_RECEIVED,
                           [](const Event &event)
                           {
                               // Serial.println();
                               // Serial.print("📨 RX: " + event.source);
                           });

        eventBus.subscribe(EventType::CONNECTION_STATE_CHANGED,
                           [this, status](const Event &event)
                           {
                               const auto &connectionEvent = static_cast<const TypedEvent<ConnectionStateChangedEvent> &>(event);
                               Serial.println();
                               Serial.print(status.getConnectionName(connectionEvent.data.oldState) + " ––> " + status.getConnectionName(connectionEvent.data.newState));
                           });

        eventBus.subscribe(EventType::HEATER_STATE_CHANGED,
                           [this, status](const Event &event)
                           {
                               const auto &connectionEvent = static_cast<const TypedEvent<HeaterStateChangedEvent> &>(event);
                               Serial.println();
                               Serial.print("🔄 Состояние изменено: " + status.getStateName(connectionEvent.data.oldState) + " → " + status.getStateName(connectionEvent.data.newState));
                           });

        eventBus.subscribe(EventType::APP_CONFIG_UPDATE,
                           [this, status](const Event &event)
                           {
                               const auto &configEvent = static_cast<const TypedEvent<AppConfigUpdateEvent> &>(event);

                               keepAliveTimer.setInterval(configEvent.data.config.bus.keepAliveInterval);
                           });
    }

    void processKeepAlive()
    {
        HeaterStatus status = heaterController.getStatus();
        String keepAliveCommand = getKeepAliveCommandForState(status.state);

        if (!keepAliveCommand.isEmpty() && busDriver.isConnected())
        {
            heaterController.checkWebastoStatus();
            commandManager.addPriorityCommand(keepAliveCommand, false, [this](String tx, String rx)
                                              { eventBus.publish(EventType::KEEP_ALLIVE_SENT); });
        }
    }

    String getKeepAliveCommandForState(WebastoState state)
    {
        switch (state)
        {
        case WebastoState::PARKING_HEAT:
            return WBusCommandBuilder::createKeepAliveParking();
        case WebastoState::VENTILATION:
            return WBusCommandBuilder::createKeepAliveVentilation();
        case WebastoState::SUPP_HEAT:
            return WBusCommandBuilder::createKeepAliveSupplemental();
        case WebastoState::CIRC_PUMP:
            return WBusCommandBuilder::createKeepAliveCirculationPump();
        case WebastoState::BOOST:
            return WBusCommandBuilder::createKeepAliveBoost();
        default:
            return "";
        }
    }

    void handleButton()
    {
        bool currentButtonState = digitalRead(BUTTON_PIN);

        if (currentButtonState == false && lastButtonState == true)
        {
            if (heaterController.isConnected())
            {
                heaterController.disconnect();
            }
            else
            {
                heaterController.connect();
            }

            delay(50); // Debounce
        }

        lastButtonState = currentButtonState;
    }

    void handleSerialCommands()
    {
        if (Serial.available())
        {
            String command = Serial.readString();
            command.trim();
            command.toLowerCase();

            if (command == "connect" || command == "con")
            {
                heaterController.connect();
            }
            else if (command == "disconnect" || command == "dc")
            {
                heaterController.disconnect();
            }
            else if (command == "start")
            {
                heaterController.startParkingHeat();
            }
            else if (command == "stop")
            {
                heaterController.shutdown();
            }
            else if (command == "test")
            {
                WBusCommandBuilder::generateAndPrintAllCommands();
            }
            else if (command == "wake")
            {
                busDriver.wakeUp();
            }
            else if (command == "sleep")
            {
                busDriver.sleep();
            }
            else if (command == "help" || command == "h")
            {
                printHelp();
            }
            else
            {
                // Прямая отправка команды в очередь
                heaterController.breakIfNeeded();
                commandManager.addPriorityCommand(command);
            }
        }
    }

    void printHelp()
    {
        Serial.println("\n📋 КОМАНДЫ УПРАВЛЕНИЯ:");
        Serial.println("connect/con   - подключение к Webasto");
        Serial.println("disconnect/dc - отключение от Webasto");
        Serial.println("start         - запустить паркинг-нагрев");
        Serial.println("stop          - остановить");
        Serial.println("help/h        - эта справка");
        Serial.println();
        Serial.println("🌐 Web Interface: http://" + WiFi.softAPIP().toString());
        Serial.println("========================================");
    }
};

// Глобальный экземпляр приложения
extern WebastoApplication app;