#pragma once
#include <Arduino.h>
#include "../core/EventBus.h"
#include "../application/CommandManager.h"
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
    CommandManager &commandManager;
    DeviceInfoManager &deviceInfoManager;
    SensorManager &sensorManager;
    ErrorsManager &errorsManager;
    HeaterController &heaterController;

    bool snifferModeEnabled = false;
    String lastProcessedTx;

public:
    SnifferManager(EventBus &bus, CommandManager &cmdManager, DeviceInfoManager &deviceInfoMngr,
                   SensorManager &sensorMngr, ErrorsManager &errorsMngr, HeaterController &heaterCtrl)
        : eventBus(bus), commandManager(cmdManager), deviceInfoManager(deviceInfoMngr), sensorManager(sensorMngr), errorsManager(errorsMngr), heaterController(heaterCtrl)
    {
        eventBus.subscribe(EventType::TX_RECEIVED,
                           [this](const Event &event)
                           {
                               if (snifferModeEnabled)
                               {
                                   processTxPacket(event.source);
                               }
                           });

        eventBus.subscribe(EventType::RX_RECEIVED,
                           [this](const Event &event)
                           {
                               if (snifferModeEnabled)
                               {
                                   processRxPacket(event.source);
                               }
                           });
    }

    void toggleSnifferMode()
    {
        snifferModeEnabled = !snifferModeEnabled;
        Serial.println();
        Serial.println(snifferModeEnabled ? "🔍 Режим сниффера ВКЛЮЧЕН" : "🔍 Режим сниффера ВЫКЛЮЧЕН");
    }

    bool isSnifferModeEnabled() const
    {
        return snifferModeEnabled;
    }

    // Обработка исходящих команд (TX)
    void processTxPacket(const String &tx)
    {
        String cleanTx = tx;
        cleanTx.replace(" ", "");

        if (cleanTx.length() < 8)
        {
            return;
        }

        // Структура пакета: [HEADER][LENGTH][COMMAND][DATA...][CHECKSUM]
        // HEADER занимает 2 символа (1 байт), LENGTH - 2 символа (1 байт)
        // COMMAND начинается с 4-го символа (индекс 4)
        uint8_t command = Utils::hexStringToByte(cleanTx.substring(4, 6));

        Serial.println();
        Serial.print("📤 SNIFF TX: " + tx);
        Serial.print(" [CMD: 0x" + String(command, HEX) + "]");

        // Определяем тип команды для логирования
        String commandType = identifyCommandType(command);
        if (!commandType.isEmpty())
        {
            Serial.print(" [" + commandType + "]");
        }
    }

    // Обработка входящих ответов (RX)
    void processRxPacket(const String &rx)
    {
        String cleanRx = rx;
        cleanRx.replace(" ", "");

        if (cleanRx.length() < 8)
        {
            return;
        }

        // Структура ответа: [HEADER][LENGTH][COMMAND_ACK][DATA...][CHECKSUM]
        // HEADER занимает 2 символа, LENGTH - 2 символа
        // COMMAND_ACK начинается с 4-го символа (индекс 4)
        uint8_t responseCommand = Utils::hexStringToByte(cleanRx.substring(4, 6));
        uint8_t originalCommand = responseCommand & 0x7F; // Сбрасываем бит ACK

        Serial.println();
        Serial.print("📨 SNIFF RX: " + rx);
        Serial.print(" [ACK: 0x" + String(originalCommand, HEX) + "]");

        // // Автоматически определяем и вызываем соответствующий обработчик
        bool processed = autoProcessResponse(originalCommand, cleanRx, rx);

        if (!processed)
        {
            Serial.print(" [UNKNOWN]");
        }
    }

    // Автоматическая обработка ответов на основе команды
    bool autoProcessResponse(uint8_t command, const String &cleanRx, const String &originalRx)
    {
        switch (command)
        {
        // =========================================================================
        // ОБРАБОТКА КОМАНД ЧТЕНИЯ ИНФОРМАЦИИ (0x51)
        // =========================================================================
        case WBusCommandBuilder::CMD_READ_INFO:
        {
            return processInfoResponse(cleanRx, originalRx);
        }

        // =========================================================================
        // ОБРАБОТКА КОМАНД ЧТЕНИЯ СЕНСОРОВ (0x50)
        // =========================================================================
        case WBusCommandBuilder::CMD_READ_SENSOR:
        {
            return processSensorResponse(cleanRx, originalRx);
        }

        // =========================================================================
        // ОБРАБОТКА КОМАНД ОШИБОК (0x56)
        // =========================================================================
        case WBusCommandBuilder::CMD_READ_ERRORS:
        {
            return processErrorResponse(cleanRx, originalRx);
        }

        case WBusCommandBuilder::ERROR_READ_DETAILS:
        {
            if (cleanRx.length() >= 12)
            {
                uint8_t errorCode = Utils::hexStringToByte(cleanRx.substring(8, 10));
                errorsManager.handleErrorDetailsResponse(lastProcessedTx, originalRx, errorCode);
                return true;
            }
            return false;
        }

            // =========================================================================
            // ОБРАБОТКА КОМАНД ДИАГНОСТИКИ И УПРАВЛЕНИЯ
            // =========================================================================
            // case WBusCommandBuilder::CMD_DIAGNOSTIC: {
            //     heaterController.handleDiagnosticResponse(lastProcessedTx, originalRx);
            //     return true;
            // }

        case WBusCommandBuilder::CMD_SHUTDOWN:
        {
            heaterController.handleShutdownResponse(lastProcessedTx, originalRx);
            return true;
        }

        case WBusCommandBuilder::CMD_PARK_HEAT:
        {
            // Для команд управления извлекаем параметры из последнего TX
            int minutes = extractMinutesFromTx(lastProcessedTx);
            heaterController.handleStartParkingHeatResponse(lastProcessedTx, originalRx, minutes);
            return true;
        }

        case WBusCommandBuilder::CMD_VENTILATE:
        {
            int minutes = extractMinutesFromTx(lastProcessedTx);
            heaterController.handleStartVentilationResponse(lastProcessedTx, originalRx, minutes);
            return true;
        }

        case WBusCommandBuilder::CMD_SUPP_HEAT:
        {
            int minutes = extractMinutesFromTx(lastProcessedTx);
            heaterController.handleStartSupplementalHeatResponse(lastProcessedTx, originalRx, minutes);
            return true;
        }

        case WBusCommandBuilder::CMD_BOOST_MODE:
        {
            int minutes = extractMinutesFromTx(lastProcessedTx);
            heaterController.handleStartBoostModeResponse(lastProcessedTx, originalRx, minutes);
            return true;
        }

        case WBusCommandBuilder::CMD_CIRC_PUMP_CTRL:
        {
            bool enable = extractBoolFromTx(lastProcessedTx);
            heaterController.handleControlCirculationPumpResponse(lastProcessedTx, originalRx, enable);
            return true;
        }

        // =========================================================================
        // ОБРАБОТКА КОМАНД ТЕСТИРОВАНИЯ (0x45)
        // =========================================================================
        case WBusCommandBuilder::CMD_TEST_COMPONENT:
        {
            return processTestComponentResponse(cleanRx, originalRx);
        }

        default:
            return false;
        }
    }

private:
    // Обработка ответов на команды чтения информации (0x51)
    bool processInfoResponse(const String &cleanRx, const String &originalRx)
    {
        if (cleanRx.length() < 8)
            return false;

        uint8_t infoIndex = Utils::hexStringToByte(cleanRx.substring(6, 8));

        switch (infoIndex)
        {
        case WBusCommandBuilder::INFO_WBUS_VERSION:
            deviceInfoManager.handleWBusVersionResponse(lastProcessedTx, originalRx);
            return true;

        case WBusCommandBuilder::INFO_DEVICE_NAME:
            deviceInfoManager.handleDeviceNameResponse(lastProcessedTx, originalRx);
            return true;

        case WBusCommandBuilder::INFO_WBUS_CODE:
            deviceInfoManager.handleWBusCodeResponse(lastProcessedTx, originalRx);
            return true;

        case WBusCommandBuilder::INFO_DEVICE_ID:
            deviceInfoManager.handleDeviceIDResponse(lastProcessedTx, originalRx);
            return true;

        case WBusCommandBuilder::INFO_CTRL_MFG_DATE:
            deviceInfoManager.handleControllerManufactureDateResponse(lastProcessedTx, originalRx);
            return true;

        case WBusCommandBuilder::INFO_HEATER_MFG_DATE:
            deviceInfoManager.handleHeaterManufactureDateResponse(lastProcessedTx, originalRx);
            return true;

        case WBusCommandBuilder::INFO_CUSTOMER_ID:
            deviceInfoManager.handleCustomerIDResponse(lastProcessedTx, originalRx);
            return true;

        case WBusCommandBuilder::INFO_SERIAL_NUMBER:
            deviceInfoManager.handleSerialNumberResponse(lastProcessedTx, originalRx);
            return true;

        default:
            return false;
        }
    }

    // Обработка ответов на команды чтения сенсоров (0x50)
    bool processSensorResponse(const String &cleanRx, const String &originalRx)
    {
        if (cleanRx.length() < 8)
            return false;

        uint8_t sensorIndex = Utils::hexStringToByte(cleanRx.substring(6, 8));

        switch (sensorIndex)
        {
        case WBusCommandBuilder::SENSOR_OPERATIONAL:
            sensorManager.handleOperationalInfoResponse(lastProcessedTx, originalRx);
            return true;

        case WBusCommandBuilder::SENSOR_ON_OFF_FLAGS:
            sensorManager.handleOnOffFlagsResponse(lastProcessedTx, originalRx);
            return true;

        case WBusCommandBuilder::SENSOR_STATUS_FLAGS:
            sensorManager.handleStatusFlagsResponse(lastProcessedTx, originalRx);
            return true;

        case WBusCommandBuilder::SENSOR_OPERATING_STATE:
            sensorManager.handleOperatingStateResponse(lastProcessedTx, originalRx);
            return true;

        case WBusCommandBuilder::SENSOR_SUBSYSTEMS_STATUS:
            sensorManager.handleSubsystemsStatusResponse(lastProcessedTx, originalRx);
            return true;

        case WBusCommandBuilder::SENSOR_FUEL_SETTINGS:
            sensorManager.handleFuelSettingsResponse(lastProcessedTx, originalRx);
            return true;

        case WBusCommandBuilder::SENSOR_FUEL_PREWARMING:
            sensorManager.handleFuelPrewarmingResponse(lastProcessedTx, originalRx);
            return true;

        case WBusCommandBuilder::SENSOR_OPERATING_TIMES:
            sensorManager.handleOperatingTimesResponse(lastProcessedTx, originalRx);
            return true;

        case WBusCommandBuilder::SENSOR_BURNING_DURATION:
            sensorManager.handleBurningDurationResponse(lastProcessedTx, originalRx);
            return true;

        case WBusCommandBuilder::SENSOR_START_COUNTERS:
            sensorManager.handleStartCountersResponse(lastProcessedTx, originalRx);
            return true;

        default:
            return false;
        }
    }

    // Обработка ответов на команды ошибок (0x56)
    bool processErrorResponse(const String &cleanRx, const String &originalRx)
    {
        if (cleanRx.length() < 8)
            return false;

        uint8_t errorIndex = Utils::hexStringToByte(cleanRx.substring(6, 8));

        switch (errorIndex)
        {
        case WBusCommandBuilder::ERROR_READ_LIST:
            errorsManager.handleCheckErrorsResponse(lastProcessedTx, originalRx);
            return true;

        case WBusCommandBuilder::ERROR_CLEAR:
            errorsManager.handleResetErrorsResponse(lastProcessedTx, originalRx);
            return true;

        default:
            return false;
        }
    }

    // Обработка ответов на команды тестирования компонентов (0x45)
    bool processTestComponentResponse(const String &cleanRx, const String &originalRx)
    {
        // Для тестовых команд анализируем исходный TX для извлечения параметров
        String cleanTx = lastProcessedTx;
        cleanTx.replace(" ", "");

        if (cleanTx.length() < 16)
            return false; // Минимальная длина тестовой команды

        uint8_t component = Utils::hexStringToByte(cleanTx.substring(8, 10));
        uint8_t seconds = Utils::hexStringToByte(cleanTx.substring(10, 12));

        // Для компонентов с мощностью/частотой извлекаем дополнительные параметры
        uint16_t magnitude = (Utils::hexStringToByte(cleanTx.substring(12, 14)) << 8) |
                             Utils::hexStringToByte(cleanTx.substring(14, 16));

        switch (component)
        {
        case WBusCommandBuilder::TEST_COMBUSTION_FAN:
        {
            int powerPercent = magnitude / 2;
            heaterController.handleTestCombustionFanResponse(lastProcessedTx, originalRx, seconds, powerPercent);
            return true;
        }

        case WBusCommandBuilder::TEST_FUEL_PUMP:
        {
            int frequencyHz = magnitude / 20;
            heaterController.handleTestFuelPumpResponse(lastProcessedTx, originalRx, seconds, frequencyHz);
            return true;
        }

        case WBusCommandBuilder::TEST_GLOW_PLUG:
        {
            int powerPercent = magnitude / 2;
            heaterController.handleTestGlowPlugResponse(lastProcessedTx, originalRx, seconds, powerPercent);
            return true;
        }

        case WBusCommandBuilder::TEST_CIRCULATION_PUMP:
        {
            int powerPercent = magnitude / 2;
            heaterController.handleTestCirculationPumpResponse(lastProcessedTx, originalRx, seconds, powerPercent);
            return true;
        }

        case WBusCommandBuilder::TEST_VEHICLE_FAN:
        {
            heaterController.handleTestVehicleFanResponse(lastProcessedTx, originalRx, seconds);
            return true;
        }

        case WBusCommandBuilder::TEST_SOLENOID_VALVE:
        {
            heaterController.handleTestSolenoidValveResponse(lastProcessedTx, originalRx, seconds);
            return true;
        }

        case WBusCommandBuilder::TEST_FUEL_PREHEATING:
        {
            int powerPercent = magnitude / 2;
            heaterController.handleTestFuelPreheatingResponse(lastProcessedTx, originalRx, seconds, powerPercent);
            return true;
        }

        default:
            return false;
        }
    }

    // Вспомогательные методы для извлечения параметров из TX
    int extractMinutesFromTx(const String &tx)
    {
        String cleanTx = tx;
        cleanTx.replace(" ", "");
        if (cleanTx.length() >= 10)
        {
            return Utils::hexStringToByte(cleanTx.substring(8, 10));
        }
        return 60; // Значение по умолчанию
    }

    bool extractBoolFromTx(const String &tx)
    {
        String cleanTx = tx;
        cleanTx.replace(" ", "");
        if (cleanTx.length() >= 8)
        {
            return Utils::hexStringToByte(cleanTx.substring(8, 10)) == 0x01;
        }
        return false;
    }

    String identifyCommandType(uint8_t command)
    {
        switch (command)
        {
        case WBusCommandBuilder::CMD_READ_INFO:
            return "READ_INFO";
        case WBusCommandBuilder::CMD_READ_SENSOR:
            return "READ_SENSOR";
        case WBusCommandBuilder::CMD_READ_ERRORS:
            return "READ_ERRORS";
        case WBusCommandBuilder::CMD_DIAGNOSTIC:
            return "DIAGNOSTIC";
        case WBusCommandBuilder::CMD_SHUTDOWN:
            return "SHUTDOWN";
        case WBusCommandBuilder::CMD_PARK_HEAT:
            return "PARK_HEAT";
        case WBusCommandBuilder::CMD_VENTILATE:
            return "VENTILATION";
        case WBusCommandBuilder::CMD_SUPP_HEAT:
            return "SUPP_HEAT";
        case WBusCommandBuilder::CMD_BOOST_MODE:
            return "BOOST_MODE";
        case WBusCommandBuilder::CMD_CIRC_PUMP_CTRL:
            return "CIRC_PUMP";
        case WBusCommandBuilder::CMD_TEST_COMPONENT:
            return "TEST_COMPONENT";
        default:
            return "";
        }
    }
};