#pragma once
#include <WiFi.h>
#include "core/EventBus.h"
#include "core/ConfigManager.h"
#include "infrastructure/hardware/TJA1020Driver.h"
#include "infrastructure/network/WebSocketServer.h"
#include "infrastructure/network/ApiServer.h"
#include "application/CommandManager.h"
#include "application/SensorManager.h"
#include "application/HeaterController.h"
#include "application/DeviceInfoManager.h"
#include "application/ErrorsManager.h"
#include "application/CommandReceiver.h"
#include "common/Utils.h"
#include "common/Constants.h"

Timer keepAliveTimer(15000);

class WebastoApplication {
private:
    EventBus& eventBus;
    ConfigManager& configManager;
    
    // Аппаратный слой
    TJA1020Driver busDriver;
    
        // Приемник W-Bus пакетов
    CommanReceiver commanReceiver;
    // Управление командами
    CommandManager commandManager;
    
    // Бизнес-логика
    DeviceInfoManager deviceInfoManager;
    SensorManager sensorManager;
    ErrorsManager errorsManager;
    HeaterController heaterController;
    
    // Сетевой слой
    WebSocketServer webSocketServer;
    ApiServer apiServer;

    
    // Состояние приложения
    bool initialized = false;
    
    // Кнопка управления (пин 0)
    static const int BUTTON_PIN = 0;
    bool lastButtonState = true;

public:
    WebastoApplication() 
        : eventBus(EventBus::getInstance())
        , configManager(ConfigManager::getInstance())
        , busDriver(KLineSerial, eventBus)
        , commanReceiver(KLineSerial, eventBus)
        , commandManager(eventBus, busDriver, commanReceiver)
        , deviceInfoManager(eventBus, commandManager)
        , sensorManager(eventBus, commandManager)
        , errorsManager(eventBus, commandManager)
        , heaterController(eventBus, commandManager, busDriver, deviceInfoManager, sensorManager, errorsManager)
        , webSocketServer(eventBus, configManager.getConfig().network.wsPort)
        , apiServer(deviceInfoManager, sensorManager, errorsManager, heaterController, configManager.getConfig().network.webPort) 
    {
        // Настраиваем таймауты как в оригинальном коде
        commandManager.setTimeout(2000);
        commandManager.setInterval(150);
    }
    
    void initialize() {
        Serial.begin(115200);
        Serial.println();
        Serial.println("🚗 Webasto W-Bus Controller - Новая архитектура");
        Serial.println("===============================================");
        
        // Инициализация конфигурации
        if (!configManager.loadConfig()) {
            Serial.println("⚠️  Using default configuration");
        }
        configManager.printConfig();
        
        // Инициализация WiFi
        setupWiFi();
        
        // Инициализация аппаратного обеспечения
        busDriver.initialize();
        heaterController.initialize();

        webSocketServer.initialize();
        apiServer.initialize();
        
        // Настройка обработчиков событий
        setupEventHandlers();
  
        busDriver.connect();
        
        initialized = true;
        Serial.println();
        Serial.println("✅ Webasto Application initialized successfully");
        Serial.println("📱 Connect to: http://" + WiFi.softAPIP().toString());
        
        printHelp();
    }
    
    void process() {
        if (!initialized) return;

        commanReceiver.process();
        commandManager.process();
        // Keep-alive логика
        if (keepAliveTimer.isReady()) {
            processKeepAlive();
        }
        
        // Обработка serial команд
        handleSerialCommands();
        
        // Обработка кнопки
        handleButton();
        
        // Сетевые сервисы
        webSocketServer.process();
        apiServer.process();
        
        delay(1);
    }
    
    void printStatus() {
        HeaterStatus status = heaterController.getStatus();
        
        Serial.println();
        Serial.println("📊 Current Status:");
        Serial.println("  Heater: " + status.getStateName());
        Serial.println("  Connection: " + status.getConnectionName());
        Serial.println("  Pending commands: " + String(commandManager.getPendingCount()));
        Serial.println("  Waiting response: " + String(commandManager.isWaitingForResponse() ? "Yes" : "No"));
        Serial.println("  WebSocket clients: " + String(webSocketServer.isWebSocketConnected() ? "Connected" : "None"));
        
        if (commandManager.isWaitingForResponse()) {
            Serial.println("  Current TX: " + commandManager.getCurrentTx());
        }
    }

private:
    void setupWiFi() {
        const NetworkConfig& netConfig = configManager.getConfig().network;
        
        Serial.println();
        Serial.println("📡 Starting Access Point...");
        Serial.println("  SSID: " + netConfig.ssid);
        Serial.println("  Password: " + netConfig.password);
        
        WiFi.mode(WIFI_AP);
        bool apStarted = WiFi.softAP(netConfig.ssid, netConfig.password);
        
        if (apStarted) {
            Serial.println("✅ Access Point started");
            Serial.println("  IP: " + WiFi.softAPIP().toString());
            Serial.println("  MAC: " + WiFi.softAPmacAddress());
        } else {
            Serial.println("❌ Failed to start Access Point");
            while (true) {
                delay(1000);
            }
        }
    }
    
    void setupEventHandlers() {
        HeaterStatus status;

        // eventBus.subscribe(EventType::TX_RECEIVED,
        //     [](const Event& event) {
        //         Serial.println();
        //         Serial.print("📤 TX: " + event.source);
        //     });

        // eventBus.subscribe(EventType::RX_RECEIVED,
        //     [](const Event& event) {
        //         Serial.println();
        //         Serial.print("📨 RX: " + event.source);
        //     });

        eventBus.subscribe(EventType::CONNECTION_STATE_CHANGED,
            [this, status](const Event& event) {
    
                const auto& connectionEvent = static_cast<const TypedEvent<ConnectionStateChangedEvent>&>(event);
                Serial.println();
                Serial.print(status.getConnectionName(connectionEvent.data.oldState) + " ––> " + status.getConnectionName(connectionEvent.data.newState));
            });

        eventBus.subscribe(EventType::HEATER_STATE_CHANGED,
            [this, status](const Event& event) {
    
                const auto& connectionEvent = static_cast<const TypedEvent<HeaterStateChangedEvent>&>(event);
            Serial.println();
            Serial.print("🔄 Состояние изменено: " + status.getStateName(connectionEvent.data.oldState) + " → " + status.getStateName(connectionEvent.data.newState));
            });
    }
    
    void processKeepAlive() {
        HeaterStatus status = heaterController.getStatus();
        String keepAliveCommand = getKeepAliveCommandForState(status.state);
        
        if (!keepAliveCommand.isEmpty() && busDriver.isConnected()) {
            heaterController.checkWebastoStatus();
            commandManager.addCommand(keepAliveCommand, [this](String tx, String rx) {
                eventBus.publish(EventType::KEEP_ALLIVE_SENT);
            });
        }
    }
    
    String getKeepAliveCommandForState(WebastoState state) {
        switch (state) {
            case WebastoState::PARKING_HEAT: return WBusProtocol::CMD_KEEPALIVE_PARKING;
            case WebastoState::VENTILATION: return WBusProtocol::CMD_KEEPALIVE_VENT;
            case WebastoState::SUPP_HEAT: return WBusProtocol::CMD_KEEPALIVE_SUPP_HEAT;
            case WebastoState::CIRC_PUMP: return WBusProtocol::CMD_KEEPALIVE_CIRC_PUMP;
            case WebastoState::BOOST: return WBusProtocol::CMD_KEEPALIVE_BOOST;
            default: return "";
        }
    }
    
    void handleButton() {
        bool currentButtonState = digitalRead(BUTTON_PIN);
        
        if (currentButtonState == false && lastButtonState == true) {
            if (heaterController.isConnected()) {
                heaterController.disconnect();
            } else {
                heaterController.connect();
            }
            
            delay(50); // Debounce
        }
        
        lastButtonState = currentButtonState;
    }
    
    void handleSerialCommands() {
        if (Serial.available()) {
            String command = Serial.readString();
            command.trim();
            command.toLowerCase();
            
            if (command == "status") {
                printStatus();
            } else if (command == "connect" || command == "con") {
                heaterController.connect();
            } else if (command == "disconnect" || command == "dc") {
                heaterController.disconnect();
            } else if (command == "start") {
                heaterController.startParkingHeat();
            } else if (command == "stop") {
                heaterController.shutdown();
            } else if (command == "info" || command == "i") {
                deviceInfoManager.printInfo();
            } else if (command == "sensors") {
                sensorManager.printSensorData();
            } else if (command == "errors" || command == "err") {
                errorsManager.printErrors();
            } else if (command == "clear" || command == "clr") {
                heaterController.breakIfNeeded();
                errorsManager.resetErrors();
            } else if (command == "queue") {
                commandManager.printQueue();
            } else if (command == "help" || command == "h") {
                printHelp();
            } else {
                // Прямая отправка команды в очередь
                heaterController.breakIfNeeded();
                commandManager.addCommand(command);
            }
        }
    }
    
    void printHelp() {
        Serial.println("\n📋 КОМАНДЫ УПРАВЛЕНИЯ:");
        Serial.println("status        - текущий статус");
        Serial.println("connect/con   - подключение к Webasto");
        Serial.println("disconnect/dc - отключение от Webasto");
        Serial.println("start         - запустить паркинг-нагрев");
        Serial.println("stop          - остановить");
        Serial.println("info/i        - информация о Webasto");
        Serial.println("sensors       - данные датчиков");
        Serial.println("errors/err    - чтение ошибок");
        Serial.println("clear/clr     - стереть ошибки");
        Serial.println("log           - вкл/выкл логирование");
        Serial.println("queue         - показать очередь команд");
        Serial.println("help/h        - эта справка");
        Serial.println();
        Serial.println("🌐 Web Interface: http://" + WiFi.softAPIP().toString());
        Serial.println("========================================");
    }
};

// Глобальный экземпляр приложения
extern WebastoApplication app;