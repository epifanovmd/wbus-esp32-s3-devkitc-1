#pragma once
#include "../interfaces/IHeaterController.h"
#include "../core/EventBus.h"
#include "../core/ConfigManager.h"
#include "../infrastructure/protocol/WBusProtocol.h"
#include "../application/CommandManager.h"
#include "../application/DeviceInfoManager.h"
#include "../application/SensorManager.h"
#include "../application/ErrorsManager.h"
#include "../interfaces/IBusManager.h"
#include "../domain/Events.h" 

class HeaterController : public IHeaterController {
private:
    EventBus& eventBus;
    CommandManager& commandManager;
    IBusManager& busManager;
    DeviceInfoManager& deviceInfoManager;
    SensorManager& sensorManager;
    ErrorsManager& errorsManager;
    
    HeaterStatus currentStatus;

public:
    HeaterController(EventBus& bus, CommandManager& cmdManager, IBusManager& busMgr, DeviceInfoManager& deviceInfoMngr, SensorManager& sensorMngr, ErrorsManager& errorsMngr) 
        : eventBus(bus)
        , commandManager(cmdManager)
        , busManager(busMgr)
        , deviceInfoManager(deviceInfoMngr)
        , sensorManager(sensorMngr) 
        , errorsManager(errorsMngr)
    {
        currentStatus.state = WebastoState::OFF;
        currentStatus.connection = ConnectionState::DISCONNECTED;
    }
    
    void initialize() override {
         Serial.println();
        Serial.println("✅ Heater Controller initialized");
        neopixelWrite(RGB_PIN, 0, 0, 0);

        eventBus.subscribe(EventType::COMMAND_SENT_ERRROR, [this](const Event& event) {
            Serial.println();
            Serial.print("COMMAND_SENT_ERRROR");
            setState(WebastoState::OFF);
            setConnectionState(ConnectionState::DISCONNECTED);
        });
    }
    
    // =========================================================================
    // УПРАВЛЕНИЕ ПОДКЛЮЧЕНИЕМ
    // =========================================================================
    
    void connect() override {
        if (currentStatus.connection == ConnectionState::CONNECTING) {
            Serial.println();
            Serial.println("⚠️  Подключение уже выполняется...");
            return;
        }

        setConnectionState(ConnectionState::CONNECTING);

        Serial.println();
        Serial.println("🔌 Начинаем подключение к Webasto...");

        busManager.sendBreak();
        delay(100);

        // Запрашиваем основную информацию об устройстве
        deviceInfoManager.requestWBusVersion();
        deviceInfoManager.requestDeviceName();
        deviceInfoManager.requestWBusCode();

        // Запускаем диагностику
        commandManager.addCommand(WBusProtocol::CMD_DIAGNOSTIC,
            [this](String tx, String rx) {
                handleDiagnosticResponse(tx, rx);
            });
    }

    bool isConnected() {
        return currentStatus.connection == ConnectionState::CONNECTED;
    }
    
    void disconnect() override {
        commandManager.clear();
        commandManager.setInterval(150);
        setConnectionState(ConnectionState::DISCONNECTED);
        
        Serial.println();
        Serial.println("🔌 Отключение от Webasto выполнено");
    }
    
    // =========================================================================
    // ОСНОВНЫЕ КОМАНДЫ УПРАВЛЕНИЯ
    // =========================================================================
    
    void startParkingHeat(int minutes = 60) override {
        String command = WBusProtocol::createParkHeatCommand(minutes);
        
        commandManager.addPriorityCommand(command, 
            [this, minutes](String tx, String rx) {
                if (!rx.isEmpty()) {
                    setState(WebastoState::PARKING_HEAT);
                    Serial.println();
                    Serial.println("🔥 Паркинг-нагрев запущен на " + String(minutes) + " минут");
                } else {
                    Serial.println();
                    Serial.println("❌ Ошибка запуска паркинг-нагрева");
                }
            });
    }
    
    void startVentilation(int minutes = 60) override {
        String command = WBusProtocol::createVentilateCommand(minutes);
        
        commandManager.addPriorityCommand(command,
            [this, minutes](String tx, String rx) {
                if (!rx.isEmpty()) {
                    setState(WebastoState::VENTILATION);
                    Serial.println();
                    Serial.println("💨 Вентиляция запущена на " + String(minutes) + " минут");
                } else {
                    Serial.println();
                    Serial.println("❌ Ошибка запуска вентиляции");
                }
            });
    }
    
    void startSupplementalHeat(int minutes = 60) override {
        String command = WBusProtocol::createSuppHeatCommand(minutes);
        
        commandManager.addPriorityCommand(command,
            [this, minutes](String tx, String rx) {
                if (!rx.isEmpty()) {
                    setState(WebastoState::SUPP_HEAT);
                    Serial.println();
                    Serial.println("🔥 Дополнительный нагрев запущен на " + String(minutes) + " минут");
                } else {
                    Serial.println();
                    Serial.println("❌ Ошибка запуска дополнительного нагрева");
                }
            });
    }
    
    void startBoostMode(int minutes = 60) override {
        String command = WBusProtocol::createBoostCommand(minutes);
        
        commandManager.addPriorityCommand(command,
            [this, minutes](String tx, String rx) {
                if (!rx.isEmpty()) {
                    setState(WebastoState::BOOST);
                    Serial.println();
                    Serial.println("⚡ Boost режим запущен на " + String(minutes) + " минут");
                } else {
                    Serial.println();
                    Serial.println("❌ Ошибка запуска Boost режима");
                }
            });
    }
    
    void controlCirculationPump(bool enable) override {
        String command = WBusProtocol::createCircPumpCommand(enable);
        
        commandManager.addPriorityCommand(command,
            [this, enable](String tx, String rx) {
                if (!rx.isEmpty()) {
                    setState(WebastoState::CIRC_PUMP);
                    Serial.println();
                    Serial.println(enable ? "🔛 Циркуляционный насос включен" : "🔴 Циркуляционный насос выключен");
                } else {
                    Serial.println();
                    Serial.println("❌ Ошибка управления циркуляционным насосом");
                }
            });
    }
    
    void shutdown() override {
        commandManager.addPriorityCommand(WBusProtocol::CMD_SHUTDOWN,
            [this](String tx, String rx) {
                if (!rx.isEmpty()) {
                    Serial.println();
                    Serial.println("🛑 Нагреватель выключен");
                } else {
                    Serial.println();
                    Serial.println("❌ Ошибка выключения нагревателя");
                }
            });
    }
    
    // =========================================================================
    // ТЕСТИРОВАНИЕ КОМПОНЕНТОВ
    // =========================================================================
    
    void testCombustionFan(int seconds, int powerPercent) override {
        String command = WBusProtocol::createTestCAFCommand(seconds, powerPercent);
        
        commandManager.addPriorityCommand(command,
            [this, seconds, powerPercent](String tx, String rx) {
                if (!rx.isEmpty()) {
                    eventBus.publish(EventType::TEST_COMBUSTION_FAN_STARTED);
                    Serial.println();
                    Serial.println("🌀 Тест вентилятора горения: " + String(seconds) + "сек, " + String(powerPercent) + "%");
                } else {
                    Serial.println();
                    Serial.println("❌ Ошибка теста вентилятора горения");
                    eventBus.publish(EventType::TEST_COMBUSTION_FAN_FAILED);
                }
            });
    }
    
    void testFuelPump(int seconds, int frequencyHz) override {
        String command = WBusProtocol::createTestFuelPumpCommand(seconds, frequencyHz);
        
        commandManager.addPriorityCommand(command,
            [this, seconds, frequencyHz](String tx, String rx) {
                if (!rx.isEmpty()) {
                    eventBus.publish(EventType::TEST_FUEL_PUMP_STARTED);
                    Serial.println();
                    Serial.println("⛽ Тест топливного насоса: " + String(seconds) + "сек, " + String(frequencyHz) + "Гц");
                } else {
                    Serial.println();
                    Serial.println("❌ Ошибка теста топливного насоса");
                    eventBus.publish(EventType::TEST_FUEL_PUMP_FAILED);
                }
            });
    }
    
    void testGlowPlug(int seconds, int powerPercent) override {
        String command = WBusProtocol::createTestGlowPlugCommand(seconds, powerPercent);
        
        commandManager.addPriorityCommand(command,
            [this, seconds, powerPercent](String tx, String rx) {
                if (!rx.isEmpty()) {
                    eventBus.publish(EventType::TEST_GLOW_PLUG_STARTED);
                    Serial.println();
                    Serial.println("🔌 Тест свечи накаливания: " + String(seconds) + "сек, " + String(powerPercent) + "%");
                } else {
                    Serial.println();
                    Serial.println("❌ Ошибка теста свечи накаливания");
                    eventBus.publish(EventType::TEST_GLOW_PLUG_FAILED);
                }
            });
    }
    
    void testCirculationPump(int seconds, int powerPercent) override {
        String command = WBusProtocol::createTestCircPumpCommand(seconds, powerPercent);
        
        commandManager.addPriorityCommand(command,
            [this, seconds, powerPercent](String tx, String rx) {
                if (!rx.isEmpty()) {
                    eventBus.publish(EventType::TEST_CIRCULATION_PUMP_STARTED);
                    Serial.println();
                    Serial.println("💧 Тест циркуляционного насоса: " + String(seconds) + "сек, " + String(powerPercent) + "%");
                } else {
                    Serial.println();
                    Serial.println("❌ Ошибка теста циркуляционного насоса");
                    eventBus.publish(EventType::TEST_CIRCULATION_PUMP_FAILED);
                }
            });
    }
    
    void testVehicleFan(int seconds) override {
        String command = WBusProtocol::createTestVehicleFanCommand(seconds);
        
        commandManager.addPriorityCommand(command,
            [this, seconds](String tx, String rx) {
                if (!rx.isEmpty()) {
                    eventBus.publish(EventType::TEST_VEHICLE_FAN_STARTED);
                    Serial.println();
                    Serial.println("🌀 Тест реле вентилятора автомобиля: " + String(seconds) + "сек");
                } else {
                    Serial.println();
                    Serial.println("❌ Ошибка теста реле вентилятора автомобиля");
                    eventBus.publish(EventType::TEST_VEHICLE_FAN_FAILED);
                }
            });
    }
    
    void testSolenoidValve(int seconds) override {
        String command = WBusProtocol::createTestSolenoidCommand(seconds);
        
        commandManager.addPriorityCommand(command,
            [this, seconds](String tx, String rx) {
                if (!rx.isEmpty()) {
                    eventBus.publish(EventType::TEST_SOLENOID_STARTED);
                    Serial.println();
                    Serial.println("🔘 Тест соленоидного клапана: " + String(seconds) + "сек");
                } else {
                    Serial.println();
                    Serial.println("❌ Ошибка теста соленоидного клапана");
                    eventBus.publish(EventType::TEST_SOLENOID_FAILED);
                }
            });
    }
    
    void testFuelPreheating(int seconds, int powerPercent) override {
        String command = WBusProtocol::createTestFuelPreheatCommand(seconds, powerPercent);
        
        commandManager.addPriorityCommand(command,
            [this, seconds, powerPercent](String tx, String rx) {
                if (!rx.isEmpty()) {
                    Serial.println();
                    eventBus.publish(EventType::TEST_FUEL_PREHEATING_STARTED);
                    Serial.println("🔥 Тест подогрева топлива: " + String(seconds) + "сек, " + String(powerPercent) + "%");
                } else {
                    Serial.println();
                    Serial.println("❌ Ошибка теста подогрева топлива");
                    eventBus.publish(EventType::TEST_FUEL_PREHEATING_FAILED);
                }
            });
    }

    void checkWebastoStatus() {
        sensorManager.requestOnOffFlags(false, [this](String tx, String rx, OnOffFlags* onOff) {
            Serial.println();
            Serial.print(onOff->toJson());
            updateHeaterStateFromSensors(onOff);
        });
        sensorManager.requestStatusFlags(false, [this](String tx, String rx, StatusFlags* status) {
            updateHeaterStateFromFlags(status);
        });
    }

    HeaterStatus getStatus() const override {
        return currentStatus;
    }

private:
    void handleDiagnosticResponse(String tx, String rx) {
        if (!rx.isEmpty()) {
            Serial.println();
            Serial.println("✅ Подключение к Webasto установлено");
            setConnectionState(ConnectionState::CONNECTED);


            // Успешное подключение - запрашиваем остальную информацию
            deviceInfoManager.requestDeviceID();
            deviceInfoManager.requestControllerManufactureDate();
            deviceInfoManager.requestHeaterManufactureDate();
            deviceInfoManager.requestCustomerID();
            deviceInfoManager.requestSerialNumber();

            // Настраиваем интервал очереди как в оригинале
            commandManager.setInterval(200);

            // Запускаем периодический опрос сенсоров
            startSensorMonitoring();
            errorsManager.checkErrors(true);
        } else {
            Serial.println();
            Serial.println("❌ Ошибка подключения к Webasto");
            setConnectionState(ConnectionState::CONNECTION_FAILED);
        }
    }

    void startSensorMonitoring() {
        sensorManager.requestOperationalInfo(true);
        sensorManager.requestOnOffFlags(true);
        sensorManager.requestStatusFlags(true);
        sensorManager.requestOperatingState(true);
        sensorManager.requestSubsystemsStatus(true);
        sensorManager.requestFuelSettings();
    }

    void updateHeaterStateFromFlags(StatusFlags* flags) {
        WebastoState newState = determineStateFromFlags(flags);
        
        if (newState != currentStatus.state) {
            setState(newState);
        }
    }

    void updateHeaterStateFromSensors(OnOffFlags* onOff) {
        // Дополнительная логика определения состояния на основе активных компонентов
        if (currentStatus.state == WebastoState::CIRC_PUMP && 
            !onOff->circulationPump) {
            setState(WebastoState::OFF);
        }
    }

    WebastoState determineStateFromFlags(StatusFlags* flags) {
        if (flags->parkingHeatRequest) return WebastoState::PARKING_HEAT;
        if (flags->ventilationRequest) return WebastoState::VENTILATION;
        if (flags->supplementalHeatRequest) return WebastoState::SUPP_HEAT;
        if (flags->boostMode) return WebastoState::BOOST;
        if (flags->mainSwitch) return WebastoState::READY;
        return WebastoState::OFF;
    }

    void setState(WebastoState newState) {
        if (currentStatus.state != newState) {
            WebastoState oldState = currentStatus.state;
            currentStatus.state = newState;
            
            eventBus.publish<HeaterStateChangedEvent>(EventType::HEATER_STATE_CHANGED,{oldState, newState});
            commandManager.clear();
            
            Serial.println("🔄 Состояние изменено: " + getStateName(oldState) + " → " + getStateName(newState));
        }
    }
    
    void setConnectionState(ConnectionState newState) {
        if (currentStatus.connection != newState) {
            ConnectionState oldState = currentStatus.connection;
            currentStatus.connection = newState;

              // Дополнительные действия при изменении состояния
            switch (newState)
            {
            case ConnectionState::CONNECTING:
                neopixelWrite(RGB_PIN, 255 / 4, 165 / 4, 0);
                break;
            case ConnectionState::CONNECTION_FAILED:
                neopixelWrite(RGB_PIN, 255 / 4, 0, 0);
                break;
            case ConnectionState::CONNECTED:
                neopixelWrite(RGB_PIN, 0, 255 / 4, 0);
                break;
            case ConnectionState::DISCONNECTED:
                neopixelWrite(RGB_PIN, 0, 0, 0);
                break;
            }
            
            eventBus.publish<ConnectionStateChangedEvent>(EventType::CONNECTION_STATE_CHANGED, {oldState, newState});
        }
    }
    
    String getStateName(WebastoState state) {
        return currentStatus.getStateName(state);
    }
};