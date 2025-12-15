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
    HeaterController(
        EventBus &bus, CommandManager &cmdManager, IBusManager &busMgr, DeviceInfoManager &deviceInfoMngr, SensorManager &sensorMngr, ErrorsManager &errorsMngr)
        : eventBus(bus),
          commandManager(cmdManager),
          busManager(busMgr),
          deviceInfoManager(deviceInfoMngr),
          sensorManager(sensorMngr),
          errorsManager(errorsMngr)
    {
        currentStatus.state = WebastoState::OFF;
        currentStatus.connection = ConnectionState::DISCONNECTED;
    }

    void initialize() override
    {
        neopixelWrite(RGB_PIN, 0, 0, 0);

        eventBus.subscribe(EventType::COMMAND_SENT_ERRROR, [this](const Event &event)
                           {
        setState(WebastoState::OFF);
        setConnectionState(ConnectionState::DISCONNECTED); });

        eventBus.subscribe(EventType::SENSOR_STATUS_FLAGS, [this](const Event &event)
                           {
        const auto & statusEvent = static_cast <
          const TypedEvent < StatusFlags > & > (event);

        StatusFlags statusFlags = statusEvent.data;

        updateHeaterStateFromStatusFlags( & statusFlags); });
    }

    // =========================================================================
    // УПРАВЛЕНИЕ ПОДКЛЮЧЕНИЕМ
    // =========================================================================

    void connect() override
    {
        if (currentStatus.connection == ConnectionState::CONNECTING)
        {
            Serial.println("⚠️  Подключение уже выполняется...");
            return;
        }

        setConnectionState(ConnectionState::CONNECTING);

        busManager.sendBreak();

        // Запрашиваем основную информацию об устройстве
        deviceInfoManager.requestWBusVersion();
        deviceInfoManager.requestDeviceName();
        deviceInfoManager.requestWBusCode();

        // Запускаем диагностику
        if (!commandManager.addCommand(WBusCommandBuilder::createDiagnostic(), false, [this](String tx, String rx)
                                       { handleDiagnosticResponse(tx, rx); }))
        {
            setConnectionState(ConnectionState::DISCONNECTED);
        }
    }

    bool isConnected()
    {
        return currentStatus.connection == ConnectionState::CONNECTED;
    }

    void disconnect() override
    {
        commandManager.clear();
        setConnectionState(ConnectionState::DISCONNECTED);
    }

    // =========================================================================
    // ОСНОВНЫЕ КОМАНДЫ УПРАВЛЕНИЯ
    // =========================================================================

    void startParkingHeat(int minutes = 59) override
    {
        breakIfNeeded();

        commandManager.addPriorityCommand(WBusCommandBuilder::createParkHeat(minutes), false, [this](String tx, String rx)
                                          { sensorManager.requestStatusFlags(); });
    }

    void startVentilation(int minutes = 59) override
    {
        breakIfNeeded();

        commandManager.addPriorityCommand(WBusCommandBuilder::createVentilation(minutes), false, [this](String tx, String rx)
                                          { sensorManager.requestStatusFlags(); });
    }

    void startSupplementalHeat(int minutes = 59) override
    {
        breakIfNeeded();

        commandManager.addPriorityCommand(WBusCommandBuilder::createSupplementalHeat(minutes), false, [this](String tx, String rx)
                                          { sensorManager.requestStatusFlags(); });
    }

    void startBoostMode(int minutes = 59) override
    {
        breakIfNeeded();

        commandManager.addPriorityCommand(WBusCommandBuilder::createBoostMode(minutes), false, [this](String tx, String rx)
                                          { sensorManager.requestStatusFlags(); });
    }

    void controlCirculationPump(bool enable) override
    {
        breakIfNeeded();

        commandManager.addPriorityCommand(WBusCommandBuilder::createCirculationPumpControl(enable), false, [this](String tx, String rx)
                                          { sensorManager.requestStatusFlags(); });
    }

    void fuelCirculation(int seconds) override
    {
        breakIfNeeded();

        commandManager.addPriorityCommand(WBusCommandBuilder::createFuelCirculation(seconds), false, [this](String tx, String rx)
                                          { sensorManager.requestStatusFlags(); });
    }

    void shutdown() override
    {
        breakIfNeeded();

        commandManager.addPriorityCommand(WBusCommandBuilder::createShutdown(), false);
    }

    // =========================================================================
    // ТЕСТИРОВАНИЕ КОМПОНЕНТОВ
    // =========================================================================

    void testCombustionFan(int seconds, int powerPercent) override
    {
        breakIfNeeded();

        commandManager.addPriorityCommand(WBusCommandBuilder::createTestCombustionFan(seconds, powerPercent), false,
                                          [this](String tx, String rx)
                                          {
                                              sensorManager.requestStatusFlags();
                                          });
    }

    void testFuelPump(int seconds, int frequencyHz) override
    {
        breakIfNeeded();

        commandManager.addPriorityCommand(WBusCommandBuilder::createTestFuelPump(seconds, frequencyHz), false,
                                          [this](String tx, String rx)
                                          {
                                              sensorManager.requestStatusFlags();
                                          });
    }

    void testGlowPlug(int seconds, int powerPercent) override
    {
        breakIfNeeded();

        commandManager.addPriorityCommand(WBusCommandBuilder::createTestGlowPlug(seconds, powerPercent),
                                          false,
                                          [this](String tx, String rx)
                                          {
                                              sensorManager.requestStatusFlags();
                                          });
    }

    void testCirculationPump(int seconds) override
    {
        breakIfNeeded();

        commandManager.addPriorityCommand(WBusCommandBuilder::createTestCirculationPump(seconds), false,

                                          [this](String tx, String rx)
                                          {
                                              sensorManager.requestStatusFlags();
                                          });
    }

    void testVehicleFan(int seconds) override
    {
        breakIfNeeded();

        commandManager.addPriorityCommand(WBusCommandBuilder::createTestVehicleFan(seconds),
                                          false,
                                          [this](String tx, String rx)
                                          {
                                              sensorManager.requestStatusFlags();
                                          });
    }

    void testSolenoidValve(int seconds) override
    {
        breakIfNeeded();

        commandManager.addPriorityCommand(WBusCommandBuilder::createTestSolenoidValve(seconds),
                                          false,
                                          [this](String tx, String rx)
                                          {
                                              sensorManager.requestStatusFlags();
                                          });
    }

    void testFuelPreheating(int seconds, int powerPercent) override
    {
        breakIfNeeded();

        commandManager.addPriorityCommand(WBusCommandBuilder::createTestFuelPreheating(seconds, powerPercent),
                                          false,
                                          [this](String tx, String rx)
                                          {
                                              sensorManager.requestStatusFlags();
                                          });
    }

    void checkWebastoStatus()
    {
        breakIfNeeded();
        sensorManager.requestStatusFlags();
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
            errorsManager.checkErrors(true);
            sensorManager.requestAllSensorData(true);
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

            Serial.println("🔥 Паркинг-нагрев запущен на " + String(minutes) + " минут");
        }
        else
        {
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

            Serial.println("💨 Вентиляция запущена на " + String(minutes) + " минут");
        }
        else
        {
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

            Serial.println("🔥 Дополнительный нагрев запущен на " + String(minutes) + " минут");
        }
        else
        {
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

            Serial.println("⚡ Boost режим запущен на " + String(minutes) + " минут");
        }
        else
        {
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

            Serial.println(enable ? "🔛 Циркуляционный насос включен" : "🔴 Циркуляционный насос выключен");
        }
        else
        {
            Serial.println("❌ Ошибка управления циркуляционным насосом");
        }
    }

    void handleShutdownResponse(String tx, String rx)
    {
        if (!rx.isEmpty())
        {
            Serial.println("🛑 Нагреватель выключен");
        }
        else
        {
            Serial.println("❌ Ошибка выключения нагревателя");
        }
    }

    void handleFuelCirculation(String tx, String rx, int seconds)
    {
        if (!rx.isEmpty())
        {
            eventBus.publish(EventType::FUEL_CIRCULATION);
            Serial.println("🛑 Прокачка топлива включена: " + String(seconds) + "сек, ");
        }
        else
        {
            Serial.println("❌ Ошибка включения прокачки топлива");
        }
    }

    void handleTestCombustionFanResponse(String tx, String rx, int seconds, int powerPercent)
    {
        if (!rx.isEmpty())
        {
            eventBus.publish(EventType::TEST_COMBUSTION_FAN_STARTED);
            Serial.println("🌀 Тест вентилятора горения: " + String(seconds) + "сек, " + String(powerPercent) + "%");
        }
        else
        {

            Serial.println("❌ Ошибка теста вентилятора горения");
            eventBus.publish(EventType::TEST_COMBUSTION_FAN_FAILED);
        }
    }

    void handleTestFuelPumpResponse(String tx, String rx, int seconds, int frequencyHz)
    {
        if (!rx.isEmpty())
        {
            eventBus.publish(EventType::TEST_FUEL_PUMP_STARTED);
            Serial.println("⛽ Тест топливного насоса: " + String(seconds) + "сек, " + String(frequencyHz) + "Гц");
        }
        else
        {
            Serial.println("❌ Ошибка теста топливного насоса");
            eventBus.publish(EventType::TEST_FUEL_PUMP_FAILED);
        }
    }

    void handleTestGlowPlugResponse(String tx, String rx, int seconds, int powerPercent)
    {
        if (!rx.isEmpty())
        {
            eventBus.publish(EventType::TEST_GLOW_PLUG_STARTED);
            Serial.println("🔌 Тест свечи накаливания: " + String(seconds) + "сек, " + String(powerPercent) + "%");
        }
        else
        {
            Serial.println("❌ Ошибка теста свечи накаливания");
            eventBus.publish(EventType::TEST_GLOW_PLUG_FAILED);
        }
    }

    void handleTestCirculationPumpResponse(String tx, String rx, int seconds)
    {
        if (!rx.isEmpty())
        {
            eventBus.publish(EventType::TEST_CIRCULATION_PUMP_STARTED);
            Serial.println("💧 Тест циркуляционного насоса: " + String(seconds) + "сек");
        }
        else
        {
            Serial.println("❌ Ошибка теста циркуляционного насоса");
            eventBus.publish(EventType::TEST_CIRCULATION_PUMP_FAILED);
        }
    }

    void handleTestVehicleFanResponse(String tx, String rx, int seconds)
    {
        if (!rx.isEmpty())
        {
            eventBus.publish(EventType::TEST_VEHICLE_FAN_STARTED);
            Serial.println("🌀 Тест реле вентилятора автомобиля: " + String(seconds) + "сек");
        }
        else
        {
            Serial.println("❌ Ошибка теста реле вентилятора автомобиля");
            eventBus.publish(EventType::TEST_VEHICLE_FAN_FAILED);
        }
    }

    void handleTestSolenoidValveResponse(String tx, String rx, int seconds)
    {
        if (!rx.isEmpty())
        {
            eventBus.publish(EventType::TEST_SOLENOID_STARTED);
            Serial.println("🔘 Тест соленоидного клапана: " + String(seconds) + "сек");
        }
        else
        {
            Serial.println("❌ Ошибка теста соленоидного клапана");
            eventBus.publish(EventType::TEST_SOLENOID_FAILED);
        }
    }

    void handleTestFuelPreheatingResponse(String tx, String rx, int seconds, int powerPercent)
    {
        if (!rx.isEmpty())
        {
            eventBus.publish(EventType::TEST_FUEL_PREHEATING_STARTED);
            Serial.println("🔥 Тест подогрева топлива: " + String(seconds) + "сек, " + String(powerPercent) + "%");
        }
        else
        {
            Serial.println("❌ Ошибка теста подогрева топлива");
            eventBus.publish(EventType::TEST_FUEL_PREHEATING_FAILED);
        }
    }

private:
    void updateHeaterStateFromStatusFlags(StatusFlags *status)
    {
        WebastoState newState = determineStateFromFlags(status);

        if (newState != currentStatus.state)
        {
            setState(newState);
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

            eventBus.publish<HeaterStateChangedEvent>(EventType::HEATER_STATE_CHANGED, {oldState,
                                                                                        newState});
        }
    }

    void setConnectionState(ConnectionState newState)
    {
        if (currentStatus.connection != newState)
        {
            ConnectionState oldState = currentStatus.connection;
            currentStatus.connection = newState;

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

                deviceInfoManager.clear();
                sensorManager.clear();
                errorsManager.clear();

                break;
            }

            eventBus.publish<ConnectionStateChangedEvent>(EventType::CONNECTION_STATE_CHANGED, {oldState,
                                                                                                newState});
        }
    }
};