#pragma once
#include "../interfaces/IHeaterController.h"
#include "../core/EventBus.h"
#include "../core/ConfigManager.h"
#include "../application/CommandManager.h"
#include "../application/DeviceInfoManager.h"
#include "../application/SensorManager.h"
#include "../application/ErrorsManager.h"
#include "../interfaces/IBusManager.h"
#include "../domain/Events.h"

class HeaterController : public IHeaterController
{
private:
    EventBus &eventBus;
    CommandManager &commandManager;
    IBusManager &busManager;
    DeviceInfoManager &deviceInfoManager;
    SensorManager &sensorManager;
    ErrorsManager &errorsManager;

    HeaterStatus currentStatus;

public:
    HeaterController(EventBus &bus, CommandManager &cmdManager, IBusManager &busMgr, DeviceInfoManager &deviceInfoMngr, SensorManager &sensorMngr, ErrorsManager &errorsMngr)
        : eventBus(bus), commandManager(cmdManager), busManager(busMgr), deviceInfoManager(deviceInfoMngr), sensorManager(sensorMngr), errorsManager(errorsMngr)
    {
        currentStatus.state = WebastoState::OFF;
        currentStatus.connection = ConnectionState::DISCONNECTED;
    }

    void initialize() override
    {
        Serial.println();
        Serial.println("✅ Heater Controller initialized");
        neopixelWrite(RGB_PIN, 0, 0, 0);

        eventBus.subscribe(EventType::COMMAND_SENT_ERRROR, [this](const Event &event)
                           {
            setState(WebastoState::OFF);
            setConnectionState(ConnectionState::DISCONNECTED); });
    }

    // =========================================================================
    // УПРАВЛЕНИЕ ПОДКЛЮЧЕНИЕМ
    // =========================================================================

    void connect() override
    {
        if (currentStatus.connection == ConnectionState::CONNECTING)
        {
            Serial.println();
            Serial.println("⚠️  Подключение уже выполняется...");
            return;
        }

        setConnectionState(ConnectionState::CONNECTING);

        busManager.sendBreak();
        delay(100);

        // Запрашиваем основную информацию об устройстве
        deviceInfoManager.requestWBusVersion();
        deviceInfoManager.requestDeviceName();
        deviceInfoManager.requestWBusCode();

        // Запускаем диагностику
        commandManager.addCommand(WBusCommandBuilder::createDiagnostic(),
                                  [this](String tx, String rx)
                                  {
                                      handleDiagnosticResponse(tx, rx);
                                  });
    }

    bool isConnected()
    {
        return currentStatus.connection == ConnectionState::CONNECTED;
    }

    void disconnect() override
    {
        commandManager.clear();
        commandManager.setInterval(150);
        setConnectionState(ConnectionState::DISCONNECTED);
    }

    // =========================================================================
    // ОСНОВНЫЕ КОМАНДЫ УПРАВЛЕНИЯ
    // =========================================================================

    void startParkingHeat(int minutes = 60) override
    {
        String command = WBusCommandBuilder::createParkHeat(minutes);

        breakIfNeeded();

        commandManager.addCommand(command,
                                  [this, minutes](String tx, String rx)
                                  {
                                      handleStartParkingHeatResponse(tx, rx, minutes);
                                  });
    }

    void startVentilation(int minutes = 60) override
    {
        String command = WBusCommandBuilder::createVentilation(minutes);

        breakIfNeeded();

        commandManager.addCommand(command,
                                  [this, minutes](String tx, String rx)
                                  {
                                      handleStartVentilationResponse(tx, rx, minutes);
                                  });
    }

    void startSupplementalHeat(int minutes = 60) override
    {
        String command = WBusCommandBuilder::createSupplementalHeat(minutes);

        breakIfNeeded();

        commandManager.addCommand(command,
                                  [this, minutes](String tx, String rx)
                                  {
                                      handleStartSupplementalHeatResponse(tx, rx, minutes);
                                  });
    }

    void startBoostMode(int minutes = 60) override
    {
        String command = WBusCommandBuilder::createBoostMode(minutes);

        breakIfNeeded();

        commandManager.addCommand(command,
                                  [this, minutes](String tx, String rx)
                                  {
                                      handleStartBoostModeResponse(tx, rx, minutes);
                                  });
    }

    void controlCirculationPump(bool enable) override
    {
        String command = WBusCommandBuilder::createCirculationPumpControl(enable);

        breakIfNeeded();

        commandManager.addCommand(command,
                                  [this, enable](String tx, String rx)
                                  {
                                      handleControlCirculationPumpResponse(tx, rx, enable);
                                  });
    }

    void shutdown() override
    {
        breakIfNeeded();

        commandManager.addCommand(WBusCommandBuilder::createShutdown(),
                                  [this](String tx, String rx)
                                  {
                                      handleShutdownResponse(tx, rx);
                                  });
    }

    // =========================================================================
    // ТЕСТИРОВАНИЕ КОМПОНЕНТОВ
    // =========================================================================

    void testCombustionFan(int seconds, int powerPercent) override
    {
        String command = WBusCommandBuilder::createTestCombustionFan(seconds, powerPercent);

        breakIfNeeded();

        commandManager.addCommand(command,
                                  [this, seconds, powerPercent](String tx, String rx)
                                  {
                                      handleTestCombustionFanResponse(tx, rx, seconds, powerPercent);
                                  });
    }

    void testFuelPump(int seconds, int frequencyHz) override
    {
        String command = WBusCommandBuilder::createTestFuelPump(seconds, frequencyHz);

        breakIfNeeded();

        commandManager.addCommand(command,
                                  [this, seconds, frequencyHz](String tx, String rx)
                                  {
                                      handleTestFuelPumpResponse(tx, rx, seconds, frequencyHz);
                                  });
    }

    void testGlowPlug(int seconds, int powerPercent) override
    {
        String command = WBusCommandBuilder::createTestGlowPlug(seconds, powerPercent);

        breakIfNeeded();

        commandManager.addCommand(command,
                                  [this, seconds, powerPercent](String tx, String rx)
                                  {
                                      handleTestGlowPlugResponse(tx, rx, seconds, powerPercent);
                                  });
    }

    void testCirculationPump(int seconds, int powerPercent) override
    {
        String command = WBusCommandBuilder::createTestCirculationPump(seconds, powerPercent);

        breakIfNeeded();

        commandManager.addCommand(command,
                                  [this, seconds, powerPercent](String tx, String rx)
                                  {
                                      handleTestCirculationPumpResponse(tx, rx, seconds, powerPercent);
                                  });
    }

    void testVehicleFan(int seconds) override
    {
        String command = WBusCommandBuilder::createTestVehicleFan(seconds);

        breakIfNeeded();

        commandManager.addCommand(command,
                                  [this, seconds](String tx, String rx)
                                  {
                                      handleTestVehicleFanResponse(tx, rx, seconds);
                                  });
    }

    void testSolenoidValve(int seconds) override
    {
        String command = WBusCommandBuilder::createTestSolenoidValve(seconds);

        breakIfNeeded();

        commandManager.addCommand(command,
                                  [this, seconds](String tx, String rx)
                                  {
                                      handleTestSolenoidValveResponse(tx, rx, seconds);
                                  });
    }

    void testFuelPreheating(int seconds, int powerPercent) override
    {
        String command = WBusCommandBuilder::createTestFuelPreheating(seconds, powerPercent);

        breakIfNeeded();

        commandManager.addCommand(command,
                                  [this, seconds, powerPercent](String tx, String rx)
                                  {
                                      handleTestFuelPreheatingResponse(tx, rx, seconds, powerPercent);
                                  });
    }

    void checkWebastoStatus()
    {
        breakIfNeeded();

        sensorManager.requestOnOffFlags(false, [this](String tx, String rx, OnOffFlags *onOff)
                                        { updateHeaterStateFromOnOffFlags(tx, rx, onOff); });
        sensorManager.requestStatusFlags(false, [this](String tx, String rx, StatusFlags *status)
                                         { updateHeaterStateFromStatusFlags(tx, rx, status); });
    }

    HeaterStatus getStatus() const override
    {
        return currentStatus;
    }

    void breakIfNeeded()
    {
        if (!isConnected())
        {
            busManager.sendBreak();
        }
    }

    // =========================================================================
    // ПУБЛИЧНЫЕ МЕТОДЫ ОБРАБОТКИ ОТВЕТОВ (для использования извне)
    // =========================================================================

    void handleDiagnosticResponse(String tx, String rx)
    {
        if (!rx.isEmpty())
        {
            setConnectionState(ConnectionState::CONNECTED);

            // Успешное подключение - запрашиваем остальную информацию
            deviceInfoManager.requestDeviceID();
            deviceInfoManager.requestControllerManufactureDate();
            deviceInfoManager.requestHeaterManufactureDate();
            deviceInfoManager.requestCustomerID();
            deviceInfoManager.requestSerialNumber();

            // Запускаем периодический опрос сенсоров
            startSensorMonitoring();
            errorsManager.checkErrors(true);
        }
        else
        {
            setConnectionState(ConnectionState::CONNECTION_FAILED);
        }
    }

    void handleStartParkingHeatResponse(String tx, String rx, int minutes)
    {
        if (!rx.isEmpty())
        {
            if (!isConnected())
            {
                setState(WebastoState::PARKING_HEAT);
            }

            Serial.println();
            Serial.println("🔥 Паркинг-нагрев запущен на " + String(minutes) + " минут");
        }
        else
        {
            Serial.println();
            Serial.println("❌ Ошибка запуска паркинг-нагрева");
        }
    }

    void handleStartVentilationResponse(String tx, String rx, int minutes)
    {
        if (!rx.isEmpty())
        {
            if (!isConnected())
            {
                setState(WebastoState::VENTILATION);
            }

            Serial.println();
            Serial.println("💨 Вентиляция запущена на " + String(minutes) + " минут");
        }
        else
        {
            Serial.println();
            Serial.println("❌ Ошибка запуска вентиляции");
        }
    }

    void handleStartSupplementalHeatResponse(String tx, String rx, int minutes)
    {
        if (!rx.isEmpty())
        {
            if (!isConnected())
            {
                setState(WebastoState::SUPP_HEAT);
            }

            Serial.println();
            Serial.println("🔥 Дополнительный нагрев запущен на " + String(minutes) + " минут");
        }
        else
        {
            Serial.println();
            Serial.println("❌ Ошибка запуска дополнительного нагрева");
        }
    }

    void handleStartBoostModeResponse(String tx, String rx, int minutes)
    {
        if (!rx.isEmpty())
        {
            if (!isConnected())
            {
                setState(WebastoState::BOOST);
            }

            Serial.println();
            Serial.println("⚡ Boost режим запущен на " + String(minutes) + " минут");
        }
        else
        {
            Serial.println();
            Serial.println("❌ Ошибка запуска Boost режима");
        }
    }

    void handleControlCirculationPumpResponse(String tx, String rx, bool enable)
    {
        if (!rx.isEmpty())
        {
            if (!isConnected())
            {
                setState(WebastoState::CIRC_PUMP);
            }

            Serial.println();
            Serial.println(enable ? "🔛 Циркуляционный насос включен" : "🔴 Циркуляционный насос выключен");
        }
        else
        {
            Serial.println();
            Serial.println("❌ Ошибка управления циркуляционным насосом");
        }
    }

    void handleShutdownResponse(String tx, String rx)
    {
        if (!rx.isEmpty())
        {
            Serial.println();
            Serial.println("🛑 Нагреватель выключен");
        }
        else
        {
            Serial.println();
            Serial.println("❌ Ошибка выключения нагревателя");
        }
    }

    void handleTestCombustionFanResponse(String tx, String rx, int seconds, int powerPercent)
    {
        if (!rx.isEmpty())
        {
            eventBus.publish(EventType::TEST_COMBUSTION_FAN_STARTED);
            Serial.println();
            Serial.println("🌀 Тест вентилятора горения: " + String(seconds) + "сек, " + String(powerPercent) + "%");
        }
        else
        {
            Serial.println();
            Serial.println("❌ Ошибка теста вентилятора горения");
            eventBus.publish(EventType::TEST_COMBUSTION_FAN_FAILED);
        }
    }

    void handleTestFuelPumpResponse(String tx, String rx, int seconds, int frequencyHz)
    {
        if (!rx.isEmpty())
        {
            eventBus.publish(EventType::TEST_FUEL_PUMP_STARTED);
            Serial.println();
            Serial.println("⛽ Тест топливного насоса: " + String(seconds) + "сек, " + String(frequencyHz) + "Гц");
        }
        else
        {
            Serial.println();
            Serial.println("❌ Ошибка теста топливного насоса");
            eventBus.publish(EventType::TEST_FUEL_PUMP_FAILED);
        }
    }

    void handleTestGlowPlugResponse(String tx, String rx, int seconds, int powerPercent)
    {
        if (!rx.isEmpty())
        {
            eventBus.publish(EventType::TEST_GLOW_PLUG_STARTED);
            Serial.println();
            Serial.println("🔌 Тест свечи накаливания: " + String(seconds) + "сек, " + String(powerPercent) + "%");
        }
        else
        {
            Serial.println();
            Serial.println("❌ Ошибка теста свечи накаливания");
            eventBus.publish(EventType::TEST_GLOW_PLUG_FAILED);
        }
    }

    void handleTestCirculationPumpResponse(String tx, String rx, int seconds, int powerPercent)
    {
        if (!rx.isEmpty())
        {
            eventBus.publish(EventType::TEST_CIRCULATION_PUMP_STARTED);
            Serial.println();
            Serial.println("💧 Тест циркуляционного насоса: " + String(seconds) + "сек, " + String(powerPercent) + "%");
        }
        else
        {
            Serial.println();
            Serial.println("❌ Ошибка теста циркуляционного насоса");
            eventBus.publish(EventType::TEST_CIRCULATION_PUMP_FAILED);
        }
    }

    void handleTestVehicleFanResponse(String tx, String rx, int seconds)
    {
        if (!rx.isEmpty())
        {
            eventBus.publish(EventType::TEST_VEHICLE_FAN_STARTED);
            Serial.println();
            Serial.println("🌀 Тест реле вентилятора автомобиля: " + String(seconds) + "сек");
        }
        else
        {
            Serial.println();
            Serial.println("❌ Ошибка теста реле вентилятора автомобиля");
            eventBus.publish(EventType::TEST_VEHICLE_FAN_FAILED);
        }
    }

    void handleTestSolenoidValveResponse(String tx, String rx, int seconds)
    {
        if (!rx.isEmpty())
        {
            eventBus.publish(EventType::TEST_SOLENOID_STARTED);
            Serial.println();
            Serial.println("🔘 Тест соленоидного клапана: " + String(seconds) + "сек");
        }
        else
        {
            Serial.println();
            Serial.println("❌ Ошибка теста соленоидного клапана");
            eventBus.publish(EventType::TEST_SOLENOID_FAILED);
        }
    }

    void handleTestFuelPreheatingResponse(String tx, String rx, int seconds, int powerPercent)
    {
        if (!rx.isEmpty())
        {
            Serial.println();
            eventBus.publish(EventType::TEST_FUEL_PREHEATING_STARTED);
            Serial.println("🔥 Тест подогрева топлива: " + String(seconds) + "сек, " + String(powerPercent) + "%");
        }
        else
        {
            Serial.println();
            Serial.println("❌ Ошибка теста подогрева топлива");
            eventBus.publish(EventType::TEST_FUEL_PREHEATING_FAILED);
        }
    }

private:
    void startSensorMonitoring()
    {
        sensorManager.requestOperationalInfo(true);
        sensorManager.requestOnOffFlags(true, [this](String tx, String rx, OnOffFlags *onOff)
                                        { updateHeaterStateFromOnOffFlags(tx, rx, onOff); });
        sensorManager.requestStatusFlags(true, [this](String tx, String rx, StatusFlags *status)
                                         { updateHeaterStateFromStatusFlags(tx, rx, status); });
        sensorManager.requestOperatingState(true);
        sensorManager.requestSubsystemsStatus(true);
        sensorManager.requestFuelPrewarming(true);
        sensorManager.requestOperatingTimes();
        sensorManager.requestBurningDuration();
        sensorManager.requestStartCounters();
        sensorManager.requestFuelSettings();
    }

    void updateHeaterStateFromStatusFlags(String tx, String rx, StatusFlags *status)
    {
        WebastoState newState = determineStateFromFlags(status);

        if (newState != currentStatus.state)
        {
            setState(newState);
        }
    }

    void updateHeaterStateFromOnOffFlags(String tx, String rx, OnOffFlags *onOff)
    {
        // Дополнительная логика определения состояния на основе активных компонентов
        if (currentStatus.state == WebastoState::CIRC_PUMP &&
            !onOff->circulationPump)
        {
            setState(WebastoState::OFF);
        }
    }

    WebastoState determineStateFromFlags(StatusFlags *flags)
    {
        if (flags->parkingHeatRequest)
            return WebastoState::PARKING_HEAT;
        if (flags->ventilationRequest)
            return WebastoState::VENTILATION;
        if (flags->supplementalHeatRequest)
            return WebastoState::SUPP_HEAT;
        if (flags->boostMode)
            return WebastoState::BOOST;
        if (flags->mainSwitch)
            return WebastoState::READY;
        return WebastoState::OFF;
    }

    void setState(WebastoState newState)
    {
        if (currentStatus.state != newState)
        {
            WebastoState oldState = currentStatus.state;
            currentStatus.state = newState;

            eventBus.publish<HeaterStateChangedEvent>(EventType::HEATER_STATE_CHANGED, {oldState, newState});
        }
    }

    void setConnectionState(ConnectionState newState)
    {
        if (currentStatus.connection != newState)
        {
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
};