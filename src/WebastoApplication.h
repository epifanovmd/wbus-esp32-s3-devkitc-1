#pragma once
#include <WiFi.h>
#include "core/EventBus.h"
#include "core/ConfigManager.h"
#include "core/FileSystemManager.h"
#include "infrastructure/hardware/TJA1020Driver.h"
#include "infrastructure/network/AsyncWebServer.h"
#include "infrastructure/network/WiFiManager.h"
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

    WiFiManager wifiManager;

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
    bool isSnifferMode = false;

    // Кнопка управления (пин 0)
    static const int BUTTON_PIN = 0;
    bool lastButtonState = true;
    unsigned long lastButtonPressTime = 0;
    bool buttonLongPressActivated = false; // Флаг, что длинное нажатие уже обработано
    Timer blinkTimeout;

public:
    WebastoApplication() : eventBus(EventBus::getInstance()),
                           fileSystemManager(),
                           configManager(eventBus, fileSystemManager), KLineSerial(1),
                           wifiManager(configManager, eventBus),
                           busDriver(configManager, KLineSerial, eventBus),
                           commandReceiver(KLineSerial, eventBus),
                           commandManager(configManager, eventBus, busDriver, commandReceiver),
                           deviceInfoManager(eventBus, commandManager),
                           sensorManager(eventBus, commandManager),
                           errorsManager(eventBus, commandManager),
                           heaterController(eventBus, commandManager, busDriver, deviceInfoManager, sensorManager, errorsManager),
                           snifferManager(eventBus, deviceInfoManager, sensorManager, errorsManager, heaterController),
                           asyncWebServer(eventBus, fileSystemManager, configManager, deviceInfoManager, sensorManager, errorsManager, heaterController),
                           keepAliveTimer(15000),
                           blinkTimeout(500)
    {
        pinMode(BUTTON_PIN, INPUT_PULLUP);
    }

    void initialize()
    {
        Serial.begin(115200);
        Serial.println();
        Serial.println("🚗 Webasto W-Bus Controller");
        Serial.println("Device ID: " + configManager.getConfig().deviceId);
        Serial.println("===============================================");

        configManager.initialize();
        keepAliveTimer.setInterval(configManager.getConfig().bus.keepAliveInterval);

        if (!wifiManager.initialize())
        {
            Serial.println("❌ WiFi initialization failed!");
            delay(1000);
            ESP.restart();
        }

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

        wifiManager.process();

        commandReceiver.process();
        commandManager.process();

        if (!isSnifferMode && keepAliveTimer.isReady())
        {
            processKeepAlive();
        }

        handleSerialCommands();
        handleButton();

        asyncWebServer.process();

        blinkLed();
        delay(1);
    }

private:
    void setupEventHandlers()
    {
        HeaterStatus status;

        eventBus.subscribe(EventType::TX_RECEIVED,
                           [](const Event &event)
                           {
                               // Serial.println("📤 TX: " + event.source);
                           });

        eventBus.subscribe(EventType::RX_RECEIVED,
                           [](const Event &event)
                           {
                               // Serial.println("📨 RX: " + event.source);
                           });

        eventBus.subscribe(EventType::CONNECTION_STATE_CHANGED,
                           [this, status](const Event &event)
                           {
                               const auto &connectionEvent = static_cast<const TypedEvent<ConnectionStateChangedEvent> &>(event);
                               Serial.println(status.getConnectionName(connectionEvent.data.oldState) + " ––> " + status.getConnectionName(connectionEvent.data.newState));
                           });

        eventBus.subscribe(EventType::HEATER_STATE_CHANGED,
                           [this, status](const Event &event)
                           {
                               const auto &connectionEvent = static_cast<const TypedEvent<HeaterStateChangedEvent> &>(event);
                               Serial.println("🔄 Состояние изменено: " + status.getStateName(connectionEvent.data.oldState) + " → " + status.getStateName(connectionEvent.data.newState));
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

        // Фиксируем начало нажатия
        if (currentButtonState == LOW && lastButtonState == HIGH)
        {
            lastButtonPressTime = millis();
            buttonLongPressActivated = false; // Сбрасываем флаг при новом нажатии
        }

        // Проверяем длинное нажатие (после 3000 мс даже без отпускания)
        if (currentButtonState == LOW && !buttonLongPressActivated)
        {
            unsigned long pressDuration = millis() - lastButtonPressTime;

            if (pressDuration > 3000) // Долгое нажатие (>3 сек)
            {
                // Включаем/выключаем режим сниффера
                isSnifferMode = !isSnifferMode;
                buttonLongPressActivated = true; // Помечаем, что обработали

                commandManager.setSnifferMode(isSnifferMode);

                if (isSnifferMode)
                {
                    Serial.println("🔍 Режим сниффера АКТИВИРОВАН");
                    if (heaterController.isConnected())
                    {
                        heaterController.disconnect();
                    }
                }
                else
                {
                    neopixelWrite(RGB_PIN, 0, 0, 0);
                    Serial.println("🔍 Режим сниффера ВЫКЛЮЧЕН");
                }
            }
        }

        // Обрабатываем отпускание кнопки (только если не было длинного нажатия)
        if (currentButtonState == HIGH && lastButtonState == LOW && !buttonLongPressActivated && !isSnifferMode)
        {
            unsigned long pressDuration = millis() - lastButtonPressTime;

            // Короткое нажатие (< сек)
            if (pressDuration < 2000)
            {
                // Переключаем подключение к Webasto
                if (heaterController.isConnected())
                {
                    heaterController.disconnect();
                }
                else
                {
                    heaterController.connect();
                }
            }
        }

        lastButtonState = currentButtonState;
    }

    void blinkLed()
    {
        if (isSnifferMode && blinkTimeout.isReady())
        {
            static bool ledState = true;

            if (ledState)
            {
                neopixelWrite(RGB_BUILTIN, 0, 0, RGB_BRIGHTNESS); // Синий
            }
            else
            {
                neopixelWrite(RGB_BUILTIN, 0, 0, 0);
            }
            ledState = !ledState;
        }
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
            else if (command == "sniffer" || command == "sniff")
            {
                isSnifferMode = !isSnifferMode;
                Serial.println(isSnifferMode ? "🔍 Режим сниффера АКТИВИРОВАН" : "🔍 Режим сниффера ВЫКЛЮЧЕН");
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
        Serial.println("sniffer       - переключить режим сниффера");
        Serial.println("help/h        - эта справка");
        Serial.println();
        Serial.println("🌐 Web Interface: http://" + WiFi.softAPIP().toString());
        Serial.println("========================================");
    }
};

// Глобальный экземпляр приложения
extern WebastoApplication app;