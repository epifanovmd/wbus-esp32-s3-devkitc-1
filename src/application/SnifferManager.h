#pragma once
#include <Arduino.h>
#include "../core/EventBus.h"
#include "../application/DeviceInfoManager.h"
#include "../application/SensorManager.h"
#include "../application/ErrorsManager.h"
#include "../application/HeaterController.h"
#include "../infrastructure/protocol/WBusCommandBuilder.h"
#include "../common/Utils.h"

class SnifferManager
{
private:
    EventBus &eventBus;
    DeviceInfoManager &deviceInfoManager;
    SensorManager &sensorManager;
    ErrorsManager &errorsManager;
    HeaterController &heaterController;

public:
    SnifferManager(EventBus &bus, DeviceInfoManager &deviceInfoMngr,
                   SensorManager &sensorMngr, ErrorsManager &errorsMngr, HeaterController &heaterCtrl)
        : eventBus(bus), deviceInfoManager(deviceInfoMngr), sensorManager(sensorMngr), errorsManager(errorsMngr), heaterController(heaterCtrl)
    {
        eventBus.subscribe(EventType::COMMAND_RECEIVED,
                           [this](const Event &event)
                           {
                               const auto &cmdEvent = static_cast<const TypedEvent<CommandReceivedEvent> &>(event);

                               String tx = cmdEvent.data.tx;
                               String rx = cmdEvent.data.rx;

                               uint8_t txCommand = Utils::extractByteFromString(tx, 2);
                               uint8_t rxCommandAsc = Utils::extractByteFromString(rx, 2);
                               uint8_t rxCommand = rxCommandAsc & 0x7F;
                               uint8_t rxIndex = Utils::extractByteFromString(rx, 3);

                               if (txCommand == rxCommand)
                               {
                                //    Serial.println();
                                //    Serial.print("📤 SNIFF TX: " + tx);
                                //    Serial.print("  ––––  ");
                                //    Serial.print("📨 SNIFF RX: " + rx);
                                //    Serial.print(" [ACK: 0x" + String(rxCommand, HEX) + "]");
                                //    Serial.print(" [" + WBusCommandBuilder::getCommandName(rxCommand) + "]");
                                //    Serial.print(" [" + WBusCommandBuilder::getIndexDisplayName(rxCommand, rxIndex) + "]");

                                   // Автоматически определяем и вызываем соответствующий обработчик
                                   bool processed = autoProcessResponse(rxCommand, tx, rx);

                                //    if (!processed)
                                //    {
                                //        Serial.print(" [UNKNOWN]");
                                //    }
                               }
                           });
    }

    // Автоматическая обработка ответов на основе команды
    bool autoProcessResponse(uint8_t command, const String &tx, const String &rx)
    {
        switch (command)
        {
        // =========================================================================
        // ОБРАБОТКА КОМАНД ЧТЕНИЯ ИНФОРМАЦИИ (0x51)
        // =========================================================================
        case WBusCommandBuilder::CMD_READ_INFO:
        {
            return processInfoResponse(tx, rx);
        }

        // =========================================================================
        // ОБРАБОТКА КОМАНД ЧТЕНИЯ СЕНСОРОВ (0x50)
        // =========================================================================
        case WBusCommandBuilder::CMD_READ_SENSOR:
        {
            return processSensorResponse(tx, rx);
        }

        // =========================================================================
        // ОБРАБОТКА КОМАНД ОШИБОК (0x56)
        // =========================================================================
        case WBusCommandBuilder::CMD_READ_ERRORS:
        {
            return processErrorResponse(tx, rx);
        }

            // =========================================================================
            // ОБРАБОТКА КОМАНД УПРАВЛЕНИЯ
            // =========================================================================

            // case WBusCommandBuilder::CMD_DIAGNOSTIC: {
            //     heaterController.handleDiagnosticResponse(lastProcessedTx, originalRx);
            //     return true;
            // }

        case WBusCommandBuilder::CMD_SHUTDOWN:
        {
            heaterController.handleShutdownResponse(tx, rx);
            return true;
        }

        case WBusCommandBuilder::CMD_PARK_HEAT:
        {
            // Для команд управления извлекаем параметры из последнего TX
            int minutes = Utils::extractByteFromString(tx, 3);
            heaterController.handleStartParkingHeatResponse(tx, rx, minutes);
            return true;
        }

        case WBusCommandBuilder::CMD_VENTILATE:
        {
            int minutes = Utils::extractByteFromString(tx, 3);
            heaterController.handleStartVentilationResponse(tx, rx, minutes);
            return true;
        }

        case WBusCommandBuilder::CMD_SUPP_HEAT:
        {
            int minutes = Utils::extractByteFromString(tx, 3);
            heaterController.handleStartSupplementalHeatResponse(tx, rx, minutes);
            return true;
        }

        case WBusCommandBuilder::CMD_BOOST_MODE:
        {
            int minutes = Utils::extractByteFromString(tx, 3);
            heaterController.handleStartBoostModeResponse(tx, rx, minutes);
            return true;
        }

        case WBusCommandBuilder::CMD_CIRC_PUMP_CTRL:
        {
            bool enable = Utils::extractByteFromString(tx, 3) != 0x00;
            heaterController.handleControlCirculationPumpResponse(tx, rx, enable);
            return true;
        }

        // =========================================================================
        // ОБРАБОТКА КОМАНД ТЕСТИРОВАНИЯ (0x45)
        // =========================================================================
        case WBusCommandBuilder::CMD_TEST_COMPONENT:
        {
            return processTestComponentResponse(tx, rx);
        }

        default:
            return false;
        }
    }

private:
    // Обработка ответов на команды чтения информации (0x51)
    bool processInfoResponse(const String &tx, const String &rx)
    {
        uint8_t infoIndex = Utils::extractByteFromString(tx, 3);

        switch (infoIndex)
        {
        case WBusCommandBuilder::INFO_WBUS_VERSION:
            deviceInfoManager.handleWBusVersionResponse(tx, rx);
            return true;

        case WBusCommandBuilder::INFO_DEVICE_NAME:
            deviceInfoManager.handleDeviceNameResponse(tx, rx);
            return true;

        case WBusCommandBuilder::INFO_WBUS_CODE:
            deviceInfoManager.handleWBusCodeResponse(tx, rx);
            return true;

        case WBusCommandBuilder::INFO_DEVICE_ID:
            deviceInfoManager.handleDeviceIDResponse(tx, rx);
            return true;

        case WBusCommandBuilder::INFO_CTRL_MFG_DATE:
            deviceInfoManager.handleControllerManufactureDateResponse(tx, rx);
            return true;

        case WBusCommandBuilder::INFO_HEATER_MFG_DATE:
            deviceInfoManager.handleHeaterManufactureDateResponse(tx, rx);
            return true;

        case WBusCommandBuilder::INFO_CUSTOMER_ID:
            deviceInfoManager.handleCustomerIDResponse(tx, rx);
            return true;

        case WBusCommandBuilder::INFO_SERIAL_NUMBER:
            deviceInfoManager.handleSerialNumberResponse(tx, rx);
            return true;

        default:
            return false;
        }
    }

    // Обработка ответов на команды чтения сенсоров (0x50)
    bool processSensorResponse(const String &tx, const String &rx)
    {
        uint8_t sensorIndex = Utils::extractByteFromString(tx, 3);

        switch (sensorIndex)
        {
        case WBusCommandBuilder::SENSOR_OPERATIONAL:
            sensorManager.handleOperationalInfoResponse(tx, rx);
            return true;

        case WBusCommandBuilder::SENSOR_ON_OFF_FLAGS:
            sensorManager.handleOnOffFlagsResponse(tx, rx);
            return true;

        case WBusCommandBuilder::SENSOR_STATUS_FLAGS:
            sensorManager.handleStatusFlagsResponse(tx, rx);
            return true;

        case WBusCommandBuilder::SENSOR_OPERATING_STATE:
            sensorManager.handleOperatingStateResponse(tx, rx);
            return true;

        case WBusCommandBuilder::SENSOR_SUBSYSTEMS_STATUS:
            sensorManager.handleSubsystemsStatusResponse(tx, rx);
            return true;

        case WBusCommandBuilder::SENSOR_FUEL_SETTINGS:
            sensorManager.handleFuelSettingsResponse(tx, rx);
            return true;

        case WBusCommandBuilder::SENSOR_OPERATING_TIMES:
            sensorManager.handleOperatingTimesResponse(tx, rx);
            return true;

        case WBusCommandBuilder::SENSOR_FUEL_PREWARMING:
            sensorManager.handleFuelPrewarmingResponse(tx, rx);
            return true;

        case WBusCommandBuilder::SENSOR_BURNING_DURATION:
            sensorManager.handleBurningDurationResponse(tx, rx);
            return true;

        case WBusCommandBuilder::SENSOR_START_COUNTERS:
            sensorManager.handleStartCountersResponse(tx, rx);
            return true;

        default:
            return false;
        }
    }

    // Обработка ответов на команды ошибок (0x56)
    bool processErrorResponse(const String &tx, const String &rx)
    {
        uint8_t errorIndex = Utils::extractByteFromString(tx, 3);

        switch (errorIndex)
        {
        case WBusCommandBuilder::ERROR_READ_LIST:
            errorsManager.handleCheckErrorsResponse(tx, rx);
            return true;

        case WBusCommandBuilder::ERROR_READ_DETAILS:
        {
            uint8_t errorCode = Utils::extractByteFromString(rx, 4);
            errorsManager.handleErrorDetailsResponse(tx, rx, errorCode);
            return true;
        }

        case WBusCommandBuilder::ERROR_CLEAR:
            errorsManager.handleResetErrorsResponse(tx, rx);
            return true;

        default:
            return false;
        }
    }

    // Обработка ответов на команды тестирования компонентов (0x45)
    bool processTestComponentResponse(const String &tx, const String &rx)
    {
        auto testInfo = TestComponentConverter::decodeTestCommand(tx);

        // Если декодер не смог разобрать команду
        if (testInfo.component == 0)
        {
            return false;
        }

        // Обрабатываем в зависимости от компонента
        switch (testInfo.component)
        {
        case WBusCommandBuilder::TEST_COMBUSTION_FAN:
        {
            int powerPercent = TestComponentConverter::combustionFanMagnitudeToPercent(testInfo.magnitude);
            heaterController.handleTestCombustionFanResponse(tx, rx, testInfo.seconds, powerPercent);
            return true;
        }

        case WBusCommandBuilder::TEST_FUEL_PUMP:
        {
            int frequencyHz = TestComponentConverter::fuelPumpMagnitudeToHz(testInfo.magnitude);
            heaterController.handleTestFuelPumpResponse(tx, rx, testInfo.seconds, frequencyHz);
            return true;
        }

        case WBusCommandBuilder::TEST_GLOW_PLUG:
        {
            int powerPercent = TestComponentConverter::glowPlugMagnitudeToPercent(testInfo.magnitude);
            heaterController.handleTestGlowPlugResponse(tx, rx, testInfo.seconds, powerPercent);
            return true;
        }

        case WBusCommandBuilder::TEST_CIRCULATION_PUMP:
        {
            int powerPercent = TestComponentConverter::circulationPumpMagnitudeToPercent(testInfo.magnitude);
            heaterController.handleTestCirculationPumpResponse(tx, rx, testInfo.seconds, powerPercent);
            return true;
        }

        case WBusCommandBuilder::TEST_VEHICLE_FAN:
        {
            heaterController.handleTestVehicleFanResponse(tx, rx, testInfo.seconds);
            return true;
        }

        case WBusCommandBuilder::TEST_SOLENOID_VALVE:
        {
            heaterController.handleTestSolenoidValveResponse(tx, rx, testInfo.seconds);
            return true;
        }

        case WBusCommandBuilder::TEST_FUEL_PREHEATING:
        {
            int powerPercent = TestComponentConverter::fuelPreheatingMagnitudeToPercent(testInfo.magnitude);
            heaterController.handleTestFuelPreheatingResponse(tx, rx, testInfo.seconds, powerPercent);
            return true;
        }

        default:
            return false;
        }
    }
};