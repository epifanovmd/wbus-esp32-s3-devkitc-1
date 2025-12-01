#pragma once
#include <Arduino.h>
#include "./TestComponentDataConverter.h"
#include "../../common/Utils.h"

class WBusCommandBuilder
{
public:
    // =========================================================================
    // БАЗОВЫЕ КОМАНДЫ
    // =========================================================================
    static const uint8_t CMD_SHUTDOWN = 0x10;
    static const uint8_t CMD_PARK_HEAT = 0x21;
    static const uint8_t CMD_VENTILATE = 0x22;
    static const uint8_t CMD_SUPP_HEAT = 0x23;
    static const uint8_t CMD_CIRC_PUMP_CTRL = 0x24;
    static const uint8_t CMD_BOOST_MODE = 0x25;
    static const uint8_t CMD_DIAGNOSTIC = 0x38;
    static const uint8_t CMD_KEEPALIVE = 0x44;
    static const uint8_t CMD_TEST_COMPONENT = 0x45;

    // =========================================================================
    // КОМАНДЫ ЧТЕНИЯ ДАННЫХ
    // =========================================================================
    static const uint8_t CMD_READ_SENSOR = 0x50;
    static const uint8_t CMD_READ_INFO = 0x51;
    static const uint8_t CMD_READ_CONFIG = 0x53;
    static const uint8_t CMD_READ_ERRORS = 0x56;
    static const uint8_t CMD_CO2_CALIBRATION = 0x57;

    // =========================================================================
    // ИНДЕКСЫ СЕНСОРОВ (0x50)
    // =========================================================================
    static const uint8_t SENSOR_STATUS_FLAGS = 0x02;
    static const uint8_t SENSOR_ON_OFF_FLAGS = 0x03;
    static const uint8_t SENSOR_FUEL_SETTINGS = 0x04;
    static const uint8_t SENSOR_OPERATIONAL = 0x05;
    static const uint8_t SENSOR_OPERATING_STATE = 0x06;
    static const uint8_t SENSOR_OPERATING_TIMES = 0x07;
    static const uint8_t SENSOR_BURNING_DURATION = 0x0A;
    static const uint8_t SENSOR_START_COUNTERS = 0x0C;
    static const uint8_t SENSOR_SUBSYSTEMS_STATUS = 0x0F;
    static const uint8_t SENSOR_TEMPERATURE_THRESHOLDS = 0x11;
    static const uint8_t SENSOR_VENTILATION_DURATION = 0x12;
    static const uint8_t SENSOR_FUEL_PREWARMING = 0x13;
    static const uint8_t SENSOR_SPARK_TRANSMISSION = 0x14;

    // =========================================================================
    // ИНДЕКСЫ ИНФОРМАЦИИ (0x51)
    // =========================================================================
    static const uint8_t INFO_DEVICE_ID = 0x01;
    static const uint8_t INFO_HARDWARE_VERSION = 0x02;
    static const uint8_t INFO_DATASET_ID = 0x03;
    static const uint8_t INFO_CTRL_MFG_DATE = 0x04;
    static const uint8_t INFO_HEATER_MFG_DATE = 0x05;
    static const uint8_t INFO_UNKNOWN_06 = 0x06;
    static const uint8_t INFO_CUSTOMER_ID = 0x07;
    static const uint8_t INFO_SERIAL_NUMBER = 0x09;
    static const uint8_t INFO_WBUS_VERSION = 0x0A;
    static const uint8_t INFO_DEVICE_NAME = 0x0B;
    static const uint8_t INFO_WBUS_CODE = 0x0C;
    static const uint8_t INFO_UNKNOWN_0D = 0x0D;

    // =========================================================================
    // ИНДЕКСЫ ОШИБОК (0x56)
    // =========================================================================
    static const uint8_t ERROR_READ_LIST = 0x01;
    static const uint8_t ERROR_READ_DETAILS = 0x02;
    static const uint8_t ERROR_CLEAR = 0x03;

    // =========================================================================
    // КОМПОНЕНТЫ ДЛЯ ТЕСТИРОВАНИЯ
    // =========================================================================
    static const uint8_t TEST_COMBUSTION_FAN = 0x01;
    static const uint8_t TEST_FUEL_PUMP = 0x02;
    static const uint8_t TEST_GLOW_PLUG = 0x03;
    static const uint8_t TEST_CIRCULATION_PUMP = 0x04;
    static const uint8_t TEST_VEHICLE_FAN = 0x05;
    static const uint8_t TEST_SOLENOID_VALVE = 0x09;
    static const uint8_t TEST_FUEL_PREHEATING = 0x0F;

    static String getCommandName(uint8_t command)
    {
        switch (command)
        {
        case CMD_SHUTDOWN:
            return "SHUTDOWN";
        case CMD_PARK_HEAT:
            return "PARK_HEAT";
        case CMD_VENTILATE:
            return "VENTILATION";
        case CMD_SUPP_HEAT:
            return "SUPP_HEAT";
        case CMD_CIRC_PUMP_CTRL:
            return "CIRC_PUMP_CTRL";
        case CMD_BOOST_MODE:
            return "BOOST_MODE";
        case CMD_DIAGNOSTIC:
            return "DIAGNOSTIC";
        case CMD_TEST_COMPONENT:
            return "TEST_COMPONENT";
        case CMD_READ_SENSOR:
            return "READ_SENSOR";
        case CMD_READ_INFO:
            return "READ_INFO";
        case CMD_READ_CONFIG:
            return "READ_CONFIG";
        case CMD_READ_ERRORS:
            return "READ_ERRORS";
        case CMD_CO2_CALIBRATION:
            return "CO2_CALIBRATION";
        default:
            return "UNKNOWN_0x" + String(command, HEX);
        }
    }

    static void generateAndPrintAllCommands()
    {
        Serial.println();
        Serial.println("═══════════════════════════════════════════════════════════");
        Serial.println("           ТЕСТИРОВАНИЕ ВСЕХ КОМАНД W-BUS");
        Serial.println("═══════════════════════════════════════════════════════════");
        Serial.println();

        // =========================================================================
        // БАЗОВЫЕ КОМАНДЫ УПРАВЛЕНИЯ
        // =========================================================================
        Serial.println("🚗 БАЗОВЫЕ КОМАНДЫ УПРАВЛЕНИЯ:");
        Serial.println("─────────────────────────────────────────────────────────");

        testCommand("SHUTDOWN", createShutdown(), "F4 02 10 E6");
        testCommand("DIAGNOSTIC", createDiagnostic(), "F4 02 38 CE");

        testCommand("PARK_HEAT (30min)", createParkHeat(30), "F4 03 21 1E C8");
        testCommand("PARK_HEAT (59min)", createParkHeat(59), "F4 03 21 3B ED");

        testCommand("VENTILATION (30min)", createVentilation(30), "F4 03 22 1E CB");
        testCommand("VENTILATION (59min)", createVentilation(59), "F4 03 22 3B EE");

        testCommand("SUPP_HEAT (30min)", createSupplementalHeat(30), "F4 03 23 1E CA");
        testCommand("SUPP_HEAT (59min)", createSupplementalHeat(59), "F4 03 23 3B EF");

        testCommand("BOOST_MODE (30min)", createBoostMode(30), "F4 03 25 1E CC");
        testCommand("BOOST_MODE (59min)", createBoostMode(59), "F4 03 25 3B E9");

        testCommand("CIRC_PUMP ON", createCirculationPumpControl(true), "F4 03 24 01 D2");
        testCommand("CIRC_PUMP OFF", createCirculationPumpControl(false), "F4 03 24 00 D3");

        Serial.println();

        // =========================================================================
        // KEEP-ALIVE КОМАНДЫ
        // =========================================================================
        Serial.println("🔄 KEEP-ALIVE КОМАНДЫ:");
        Serial.println("─────────────────────────────────────────────────────────");

        testCommand("KEEPALIVE PARKING", createKeepAliveParking(), "F4 04 44 21 00 95");
        testCommand("KEEPALIVE VENTILATION", createKeepAliveVentilation(), "F4 04 44 22 00 96");
        testCommand("KEEPALIVE SUPPLEMENTAL", createKeepAliveSupplemental(), "F4 04 44 23 00 97");
        testCommand("KEEPALIVE CIRC_PUMP", createKeepAliveCirculationPump(), "F4 04 44 24 00 90");
        testCommand("KEEPALIVE BOOST", createKeepAliveBoost(), "F4 04 44 25 00 91");

        Serial.println();

        // =========================================================================
        // КОМАНДЫ ЧТЕНИЯ СЕНСОРОВ (0x50)
        // =========================================================================
        Serial.println("📊 КОМАНДЫ ЧТЕНИЯ СЕНСОРОВ:");
        Serial.println("─────────────────────────────────────────────────────────");

        testCommand("SENSOR_STATUS_FLAGS", createReadSensor(SENSOR_STATUS_FLAGS), "F4 03 50 02 A5");
        testCommand("SENSOR_ON_OFF_FLAGS", createReadSensor(SENSOR_ON_OFF_FLAGS), "F4 03 50 03 A4");
        testCommand("SENSOR_FUEL_SETTINGS", createReadSensor(SENSOR_FUEL_SETTINGS), "F4 03 50 04 A3");
        testCommand("SENSOR_OPERATIONAL", createReadSensor(SENSOR_OPERATIONAL), "F4 03 50 05 A2");
        testCommand("SENSOR_OPERATING_STATE", createReadSensor(SENSOR_OPERATING_STATE), "F4 03 50 06 A1");
        testCommand("SENSOR_OPERATING_TIMES", createReadSensor(SENSOR_OPERATING_TIMES), "F4 03 50 07 A0"); // не используется
        testCommand("SENSOR_SUBSYSTEMS", createReadSensor(SENSOR_SUBSYSTEMS_STATUS), "F4 03 50 0F A8");
        testCommand("SENSOR_TEMP_THRESHOLDS", createReadSensor(SENSOR_TEMPERATURE_THRESHOLDS), "F4 03 50 11 B6"); // не используется
        testCommand("SENSOR_VENTILATION_DUR", createReadSensor(SENSOR_VENTILATION_DURATION), "F4 03 50 12 B5");   // не используется
        testCommand("SENSOR_FUEL_PREWARMING", createReadSensor(SENSOR_FUEL_PREWARMING), "F4 03 50 13 B4");        // не используется
        testCommand("SENSOR_SPARK_TRANSMISSION", createReadSensor(SENSOR_SPARK_TRANSMISSION), "F4 03 50 14 B3");  // не используется

        Serial.println();

        // =========================================================================
        // КОМАНДЫ ЧТЕНИЯ ИНФОРМАЦИИ (0x51)
        // =========================================================================
        Serial.println("ℹ️  КОМАНДЫ ЧТЕНИЯ ИНФОРМАЦИИ:");
        Serial.println("─────────────────────────────────────────────────────────");

        testCommand("INFO_DEVICE_ID", createReadInfo(INFO_DEVICE_ID), "F4 03 51 01 A7");
        testCommand("INFO_HARDWARE_VERSION", createReadInfo(INFO_HARDWARE_VERSION), "F4 03 51 02 A4"); // не используется
        testCommand("INFO_DATASET_ID", createReadInfo(INFO_DATASET_ID), "F4 03 51 03 A5");             // не используется
        testCommand("INFO_CTRL_MFG_DATE", createReadInfo(INFO_CTRL_MFG_DATE), "F4 03 51 04 A2");
        testCommand("INFO_HEATER_MFG_DATE", createReadInfo(INFO_HEATER_MFG_DATE), "F4 03 51 05 A3");
        testCommand("INFO_UNKNOWN_06", createReadInfo(INFO_UNKNOWN_06), "F4 03 51 06 A0");
        testCommand("INFO_CUSTOMER_ID", createReadInfo(INFO_CUSTOMER_ID), "F4 03 51 07 A1");
        testCommand("INFO_SERIAL_NUMBER", createReadInfo(INFO_SERIAL_NUMBER), "F4 03 51 09 AF");

        testCommand("INFO_WBUS_VERSION", createReadInfo(INFO_WBUS_VERSION), "F4 03 51 0A AC");
        testCommand("INFO_DEVICE_NAME", createReadInfo(INFO_DEVICE_NAME), "F4 03 51 0B AD");
        testCommand("INFO_WBUS_CODE", createReadInfo(INFO_WBUS_CODE), "F4 03 51 0C AA");
        testCommand("INFO_UNKNOWN_0D", createReadInfo(INFO_UNKNOWN_0D), "F4 03 51 0D AB"); // не используется

        Serial.println();

        // =========================================================================
        // КОМАНДЫ ОШИБОК (0x56)
        // =========================================================================
        Serial.println("🚨 КОМАНДЫ ОШИБОК:");
        Serial.println("─────────────────────────────────────────────────────────");

        testCommand("ERROR_READ_LIST", createReadErrors(), "F4 03 56 01 A0");
        testCommand("ERROR_CLEAR", createClearErrors(), "F4 03 56 03 A2");

        Serial.println();

        // =========================================================================
        // КОМАНДЫ ТЕСТИРОВАНИЯ КОМПОНЕНТОВ
        // =========================================================================
        Serial.println("🔧 КОМАНДЫ ТЕСТИРОВАНИЯ КОМПОНЕНТОВ:");
        Serial.println("─────────────────────────────────────────────────────────");

        testCommand("TEST_COMBUSTION_FAN (10s, 50%)",
                    createTestCombustionFan(10, 50),
                    "F4 06 45 01 0A 00 FF 43");

        testCommand("TEST_FUEL_PUMP (10s, 15Hz)",
                    createTestFuelPump(10, 15),
                    "F4 06 45 02 0A 01 2C 92");

        testCommand("TEST_GLOW_PLUG (5s, 75%)",
                    createTestGlowPlug(5, 75),
                    "F4 06 45 03 05 00 96 27");

        testCommand("TEST_CIRC_PUMP (15s, 100%)",
                    createTestCirculationPump(15, 100),
                    "F4 06 45 04 0F 00 C8 74");

        testCommand("TEST_VEHICLE_FAN (8s)",
                    createTestVehicleFan(8),
                    "F4 06 45 05 08 00 01 BB");

        testCommand("TEST_SOLENOID (12s)",
                    createTestSolenoidValve(12),
                    "F4 06 45 09 0C 00 01 B3");

        testCommand("TEST_FUEL_PREHEATING (20s, 50%)",
                    createTestFuelPreheating(20, 50),
                    "F4 06 45 0F 14 00 64 C8");

        Serial.println();
        Serial.println("═══════════════════════════════════════════════════════════");
        Serial.println("                 ТЕСТИРОВАНИЕ ЗАВЕРШЕНО");
        Serial.println("═══════════════════════════════════════════════════════════");
        Serial.println();
    }

    static void testCommand(const String &name, const String &generated, const String &expected)
    {
        Serial.print("  ");
        Serial.print(name);
        Serial.print(": ");

        if (generated == expected)
        {
            Serial.print("✅ ");
        }
        else
        {
            Serial.print("❌ ");
        }

        Serial.print(generated);

        if (generated != expected)
        {
            Serial.print(" (ожидалось: ");
            Serial.print(expected);
            Serial.print(")");
        }

        Serial.println();
    }

    // =========================================================================
    // ОСНОВНЫЕ ФУНКЦИИ СОЗДАНИЯ КОМАНД
    // =========================================================================

    // Базовая функция создания команды
    static String createCommand(uint8_t command, uint8_t index = 0, const uint8_t *data = nullptr, size_t dataLength = 0)
    {
        // Расчет длины: заголовок(1) + длина(1) + команда(1) + [индекс(1)] + [данные] + checksum(1)
        uint8_t length = 2; // команда + checksum (минимально)
        if (index != 0)
            length += 1;
        length += dataLength;

        // Буфер для данных пакета
        uint8_t packet[32];
        uint8_t packetLength = 0;

        // Заголовок
        packet[packetLength++] = 0xF4; // TX header

        // Длина
        packet[packetLength++] = length;

        // Команда
        packet[packetLength++] = command;

        // Индекс (если указан)
        if (index != 0)
        {
            packet[packetLength++] = index;
        }

        // Данные (если есть)
        if (data != nullptr && dataLength > 0)
        {
            memcpy(&packet[packetLength], data, dataLength);
            packetLength += dataLength;
        }

        // Расчет контрольной суммы
        uint8_t checksum = Utils::calculateChecksum(packet, packetLength);
        packet[packetLength++] = checksum;

        // Формирование строки
        return Utils::bytesToHexString(packet, packetLength);
    }

    // =========================================================================
    // СПЕЦИАЛИЗИРОВАННЫЕ КОМАНДЫ
    // =========================================================================

    // Команды без данных
    static String createSimpleCommand(uint8_t command)
    {
        return createCommand(command);
    }

    // Команды с одним байтом данных
    static String createCommandWithByte(uint8_t command, uint8_t dataByte)
    {
        return createCommand(command, 0, &dataByte, 1);
    }

    // Команды с индексом (без данных)
    static String createIndexedCommand(uint8_t command, uint8_t index)
    {
        return createCommand(command, index);
    }

    // =========================================================================
    // КОМАНДЫ УПРАВЛЕНИЯ
    // =========================================================================

    static String createShutdown()
    {
        return createSimpleCommand(CMD_SHUTDOWN);
    }

    static String createParkHeat(uint8_t minutes = 59)
    {
        minutes = constrain(minutes, 1, 60);
        return createCommandWithByte(CMD_PARK_HEAT, minutes);
    }

    static String createVentilation(uint8_t minutes = 59)
    {
        minutes = constrain(minutes, 1, 60);
        return createCommandWithByte(CMD_VENTILATE, minutes);
    }

    static String createSupplementalHeat(uint8_t minutes = 59)
    {
        minutes = constrain(minutes, 1, 60);
        return createCommandWithByte(CMD_SUPP_HEAT, minutes);
    }

    static String createBoostMode(uint8_t minutes = 59)
    {
        minutes = constrain(minutes, 1, 60);
        return createCommandWithByte(CMD_BOOST_MODE, minutes);
    }

    static String createCirculationPumpControl(bool enable)
    {
        uint8_t data = enable ? 0x01 : 0x00;
        return createCommandWithByte(CMD_CIRC_PUMP_CTRL, data);
    }

    static String createDiagnostic()
    {
        return createSimpleCommand(CMD_DIAGNOSTIC);
    }

    // =========================================================================
    // KEEP-ALIVE КОМАНДЫ
    // =========================================================================

    static String createKeepAlive(uint8_t mode)
    {
        uint8_t data[] = {mode, 0x00};
        return createCommand(CMD_KEEPALIVE, 0, data, 2);
    }

    static String createKeepAliveParking()
    {
        return createKeepAlive(CMD_PARK_HEAT);
    }

    static String createKeepAliveVentilation()
    {
        return createKeepAlive(CMD_VENTILATE);
    }

    static String createKeepAliveSupplemental()
    {
        return createKeepAlive(CMD_SUPP_HEAT);
    }

    static String createKeepAliveCirculationPump()
    {
        return createKeepAlive(CMD_CIRC_PUMP_CTRL);
    }

    static String createKeepAliveBoost()
    {
        return createKeepAlive(CMD_BOOST_MODE);
    }

    // =========================================================================
    // КОМАНДЫ ЧТЕНИЯ ДАННЫХ
    // =========================================================================

    // Чтение сенсоров
    static String createReadSensor(uint8_t sensorIndex)
    {
        return createIndexedCommand(CMD_READ_SENSOR, sensorIndex);
    }

    // Чтение информации
    static String createReadInfo(uint8_t infoIndex)
    {
        return createIndexedCommand(CMD_READ_INFO, infoIndex);
    }

    // Чтение ошибок
    static String createReadErrors()
    {
        return createIndexedCommand(CMD_READ_ERRORS, ERROR_READ_LIST);
    }

    static String createClearErrors()
    {
        return createIndexedCommand(CMD_READ_ERRORS, ERROR_CLEAR);
    }

    static String createReadErrorDetails(uint8_t errorCode)
    {
        uint8_t data[] = {errorCode};
        return createCommand(CMD_READ_ERRORS, ERROR_READ_DETAILS, data, 1);
    }

    // =========================================================================
    // КОМАНДЫ ТЕСТИРОВАНИЯ КОМПОНЕНТОВ
    // =========================================================================

    // Общая функция тестирования компонентов
    static String createTestCommand(uint8_t component, uint8_t seconds, uint16_t magnitude)
    {
        uint8_t data[] = {
            component,
            seconds,
            static_cast<uint8_t>(magnitude >> 8),
            static_cast<uint8_t>(magnitude & 0xFF)};
        return createCommand(CMD_TEST_COMPONENT, 0, data, 4);
    }

    // Специализированные функции тестирования с использованием новых конвертеров
    static String createTestCombustionFan(uint8_t seconds, uint8_t powerPercent)
    {
        powerPercent = constrain(powerPercent, 0, 100);
        uint16_t magnitude = TestComponentConverter::combustionFanPercentToMagnitude(powerPercent);
        return createTestCommand(TEST_COMBUSTION_FAN, seconds, magnitude);
    }

    static String createTestFuelPump(uint8_t seconds, uint8_t frequencyHz)
    {
        frequencyHz = constrain(frequencyHz, 0, 50);
        uint16_t magnitude = TestComponentConverter::fuelPumpHzToMagnitude(frequencyHz);
        return createTestCommand(TEST_FUEL_PUMP, seconds, magnitude);
    }

    static String createTestGlowPlug(uint8_t seconds, uint8_t powerPercent)
    {
        powerPercent = constrain(powerPercent, 0, 100);
        uint16_t magnitude = TestComponentConverter::glowPlugPercentToMagnitude(powerPercent);
        return createTestCommand(TEST_GLOW_PLUG, seconds, magnitude);
    }

    static String createTestCirculationPump(uint8_t seconds, uint8_t powerPercent)
    {
        powerPercent = constrain(powerPercent, 0, 100);
        uint16_t magnitude = TestComponentConverter::circulationPumpPercentToMagnitude(powerPercent);
        return createTestCommand(TEST_CIRCULATION_PUMP, seconds, magnitude);
    }

    static String createTestVehicleFan(uint8_t seconds)
    {
        uint16_t magnitude = TestComponentConverter::vehicleFanToMagnitude();
        return createTestCommand(TEST_VEHICLE_FAN, seconds, magnitude);
    }

    static String createTestSolenoidValve(uint8_t seconds)
    {
        uint16_t magnitude = TestComponentConverter::solenoidValveToMagnitude();
        return createTestCommand(TEST_SOLENOID_VALVE, seconds, magnitude);
    }

    static String createTestFuelPreheating(uint8_t seconds, uint8_t powerPercent)
    {
        powerPercent = constrain(powerPercent, 0, 100);
        uint16_t magnitude = TestComponentConverter::fuelPreheatingPercentToMagnitude(powerPercent);
        return createTestCommand(TEST_FUEL_PREHEATING, seconds, magnitude);
    }
};