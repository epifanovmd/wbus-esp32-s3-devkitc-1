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
    static const uint8_t CMD_FUEL_CIRCULATION = 0x42;
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
    static const uint8_t SENSOR_STATUS_FLAGS = 0x02;           // Статус-флаги (битовые маски)
    static const uint8_t SENSOR_ON_OFF_FLAGS = 0x03;           // Флаги вкл/выкл компонентов
    static const uint8_t SENSOR_FUEL_SETTINGS = 0x04;          // Настройки топлива (тип, время, коэфф)
    static const uint8_t SENSOR_OPERATIONAL = 0x05;            // Операционные измерения (темп, напряжение и т.д.)
    static const uint8_t SENSOR_OPERATING_TIMES = 0x06;        // Время работы (working/operating hours)
    static const uint8_t SENSOR_OPERATING_STATE = 0x07;        // Состояние работы и флаги устройства
    static const uint8_t SENSOR_BURNING_DURATION = 0x0A;       // Длительность горения по уровням мощности
    static const uint8_t SENSOR_WORKING_DURATION = 0x0B;       // Время включения PH и SH
    static const uint8_t SENSOR_START_COUNTERS = 0x0C;         // Счетчики запусков
    static const uint8_t SENSOR_SUBSYSTEMS_STATUS = 0x0F;      // Статус подсистем (свеча, насос и т.д.)
    static const uint8_t SENSOR_OTHER_DURATION = 0x10;         // Другая длительность (по описанию протокола)
    static const uint8_t SENSOR_TEMPERATURE_THRESHOLDS = 0x11; // Пороги температуры
    static const uint8_t SENSOR_VENTILATION_DURATION = 0x12;   // Длительность вентиляции
    static const uint8_t SENSOR_FUEL_PREWARMING = 0x13;        // Подогрев топлива
    static const uint8_t SENSOR_SPARK_TRANSMISSION = 0x14;     // Спарк-трансмиссия

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

    // =========================================================================
    // ФУНКЦИИ ПОЛУЧЕНИЯ ИМЕНИ ПО ИНДЕКСУ
    // =========================================================================

    // Получить имя команды по коду
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

    // Получить имя сенсора по индексу (0x50 команды)
    static String getSensorName(uint8_t sensorIndex)
    {
        switch (sensorIndex)
        {
        case SENSOR_STATUS_FLAGS:
            return "STATUS_FLAGS";
        case SENSOR_ON_OFF_FLAGS:
            return "ON_OFF_FLAGS";
        case SENSOR_FUEL_SETTINGS:
            return "FUEL_SETTINGS";
        case SENSOR_OPERATIONAL:
            return "OPERATIONAL";
        case SENSOR_OPERATING_STATE:
            return "OPERATING_STATE";
        case SENSOR_OPERATING_TIMES:
            return "OPERATING_TIMES";
        case SENSOR_BURNING_DURATION:
            return "BURNING_DURATION";
        case SENSOR_START_COUNTERS:
            return "START_COUNTERS";
        case SENSOR_SUBSYSTEMS_STATUS:
            return "SUBSYSTEMS_STATUS";
        case SENSOR_TEMPERATURE_THRESHOLDS:
            return "TEMPERATURE_THRESHOLDS";
        case SENSOR_VENTILATION_DURATION:
            return "VENTILATION_DURATION";
        case SENSOR_FUEL_PREWARMING:
            return "FUEL_PREWARMING";
        case SENSOR_SPARK_TRANSMISSION:
            return "SPARK_TRANSMISSION";
        default:
            return "UNKNOWN_SENSOR_0x" + String(sensorIndex, HEX);
        }
    }

    // Получить человекочитаемое имя сенсора
    static String getSensorDisplayName(uint8_t sensorIndex)
    {
        switch (sensorIndex)
        {
        case SENSOR_STATUS_FLAGS:
            return "Флаги состояния";
        case SENSOR_ON_OFF_FLAGS:
            return "Флаги включения/выключения";
        case SENSOR_FUEL_SETTINGS:
            return "Настройки топлива";
        case SENSOR_OPERATIONAL:
            return "Операционные измерения";
        case SENSOR_OPERATING_STATE:
            return "Состояние работы";
        case SENSOR_OPERATING_TIMES:
            return "Время работы";
        case SENSOR_BURNING_DURATION:
            return "Длительность горения";
        case SENSOR_START_COUNTERS:
            return "Счетчики запусков";
        case SENSOR_SUBSYSTEMS_STATUS:
            return "Статус подсистем";
        case SENSOR_TEMPERATURE_THRESHOLDS:
            return "Температурные пороги";
        case SENSOR_VENTILATION_DURATION:
            return "Длительность вентиляции";
        case SENSOR_FUEL_PREWARMING:
            return "Подогрев топлива";
        case SENSOR_SPARK_TRANSMISSION:
            return "Искровая передача";
        default:
            return "Неизвестный датчик (0x" + String(sensorIndex, HEX) + ")";
        }
    }

    // Получить имя информации по индексу (0x51 команды)
    static String getInfoName(uint8_t infoIndex)
    {
        switch (infoIndex)
        {
        case INFO_DEVICE_ID:
            return "DEVICE_ID";
        case INFO_HARDWARE_VERSION:
            return "HARDWARE_VERSION";
        case INFO_DATASET_ID:
            return "DATASET_ID";
        case INFO_CTRL_MFG_DATE:
            return "CONTROLLER_MANUFACTURE_DATE";
        case INFO_HEATER_MFG_DATE:
            return "HEATER_MANUFACTURE_DATE";
        case INFO_UNKNOWN_06:
            return "UNKNOWN_06";
        case INFO_CUSTOMER_ID:
            return "CUSTOMER_ID";
        case INFO_SERIAL_NUMBER:
            return "SERIAL_NUMBER";
        case INFO_WBUS_VERSION:
            return "WBUS_VERSION";
        case INFO_DEVICE_NAME:
            return "DEVICE_NAME";
        case INFO_WBUS_CODE:
            return "WBUS_CODE";
        case INFO_UNKNOWN_0D:
            return "UNKNOWN_0D";
        default:
            return "UNKNOWN_INFO_0x" + String(infoIndex, HEX);
        }
    }

    // Получить человекочитаемое имя информации
    static String getInfoDisplayName(uint8_t infoIndex)
    {
        switch (infoIndex)
        {
        case INFO_DEVICE_ID:
            return "ID устройства";
        case INFO_HARDWARE_VERSION:
            return "Версия аппаратного обеспечения";
        case INFO_DATASET_ID:
            return "ID набора данных";
        case INFO_CTRL_MFG_DATE:
            return "Дата изготовления контроллера";
        case INFO_HEATER_MFG_DATE:
            return "Дата изготовления нагревателя";
        case INFO_UNKNOWN_06:
            return "Неизвестная информация 06";
        case INFO_CUSTOMER_ID:
            return "ID заказчика";
        case INFO_SERIAL_NUMBER:
            return "Серийный номер";
        case INFO_WBUS_VERSION:
            return "Версия W-Bus";
        case INFO_DEVICE_NAME:
            return "Имя устройства";
        case INFO_WBUS_CODE:
            return "Код W-Bus";
        case INFO_UNKNOWN_0D:
            return "Неизвестная информация 0D";
        default:
            return "Неизвестная информация (0x" + String(infoIndex, HEX) + ")";
        }
    }

    // Получить имя операции с ошибками по индексу (0x56 команды)
    static String getErrorOperationName(uint8_t errorIndex)
    {
        switch (errorIndex)
        {
        case ERROR_READ_LIST:
            return "READ_ERROR_LIST";
        case ERROR_READ_DETAILS:
            return "READ_ERROR_DETAILS";
        case ERROR_CLEAR:
            return "CLEAR_ERRORS";
        default:
            return "UNKNOWN_ERROR_OP_0x" + String(errorIndex, HEX);
        }
    }

    // Получить человекочитаемое имя операции с ошибками
    static String getErrorOperationDisplayName(uint8_t errorIndex)
    {
        switch (errorIndex)
        {
        case ERROR_READ_LIST:
            return "Чтение списка ошибок";
        case ERROR_READ_DETAILS:
            return "Чтение деталей ошибки";
        case ERROR_CLEAR:
            return "Очистка ошибок";
        default:
            return "Неизвестная операция с ошибками (0x" + String(errorIndex, HEX) + ")";
        }
    }

    // Получить имя компонента для тестирования
    static String getTestComponentName(uint8_t component)
    {
        switch (component)
        {
        case TEST_COMBUSTION_FAN:
            return "COMBUSTION_FAN";
        case TEST_FUEL_PUMP:
            return "FUEL_PUMP";
        case TEST_GLOW_PLUG:
            return "GLOW_PLUG";
        case TEST_CIRCULATION_PUMP:
            return "CIRCULATION_PUMP";
        case TEST_VEHICLE_FAN:
            return "VEHICLE_FAN";
        case TEST_SOLENOID_VALVE:
            return "SOLENOID_VALVE";
        case TEST_FUEL_PREHEATING:
            return "FUEL_PREHEATING";
        default:
            return "UNKNOWN_COMPONENT_0x" + String(component, HEX);
        }
    }

    // Получить человекочитаемое имя компонента для тестирования
    static String getTestComponentDisplayName(uint8_t component)
    {
        switch (component)
        {
        case TEST_COMBUSTION_FAN:
            return "Вентилятор горения";
        case TEST_FUEL_PUMP:
            return "Топливный насос";
        case TEST_GLOW_PLUG:
            return "Свеча накаливания";
        case TEST_CIRCULATION_PUMP:
            return "Циркуляционный насос";
        case TEST_VEHICLE_FAN:
            return "Вентилятор автомобиля";
        case TEST_SOLENOID_VALVE:
            return "Соленоидный клапан";
        case TEST_FUEL_PREHEATING:
            return "Подогрев топлива";
        default:
            return "Неизвестный компонент (0x" + String(component, HEX) + ")";
        }
    }

    // Универсальная функция для получения имени по типу команды и индексу
    static String getIndexName(uint8_t command, uint8_t index)
    {
        switch (command)
        {
        case CMD_READ_SENSOR:
            return getSensorName(index);

        case CMD_READ_INFO:
            return getInfoName(index);

        case CMD_READ_ERRORS:
            return getErrorOperationName(index);

        case CMD_TEST_COMPONENT:
            return getTestComponentName(index);

        default:
            return "INDEX_0x" + String(index, HEX);
        }
    }

    // Универсальная функция для получения человекочитаемого имени
    static String getIndexDisplayName(uint8_t command, uint8_t index)
    {
        switch (command)
        {
        case CMD_READ_SENSOR:
            return getSensorDisplayName(index);

        case CMD_READ_INFO:
            return getInfoDisplayName(index);

        case CMD_READ_ERRORS:
            return getErrorOperationDisplayName(index);

        case CMD_TEST_COMPONENT:
            return getTestComponentDisplayName(index);

        default:
            return "Индекс 0x" + String(index, HEX);
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

        testCommand("SHUTDOWN", createShutdown(), "f4 02 10 e6");
        testCommand("DIAGNOSTIC", createDiagnostic(), "f4 02 38 ce");

        testCommand("PARK_HEAT (30min)", createParkHeat(30), "f4 03 21 1e c8");
        testCommand("PARK_HEAT (59min)", createParkHeat(59), "f4 03 21 3b ed");

        testCommand("VENTILATION (30min)", createVentilation(30), "f4 03 22 1e cb");
        testCommand("VENTILATION (59min)", createVentilation(59), "f4 03 22 3b ee");

        testCommand("SUPP_HEAT (30min)", createSupplementalHeat(30), "f4 03 23 1e ca");
        testCommand("SUPP_HEAT (59min)", createSupplementalHeat(59), "f4 03 23 3b ef");

        testCommand("BOOST_MODE (30min)", createBoostMode(30), "f4 03 25 1e cc");
        testCommand("BOOST_MODE (59min)", createBoostMode(59), "f4 03 25 3b e9");

        testCommand("CIRC_PUMP ON", createCirculationPumpControl(true), "f4 03 24 01 d2");
        testCommand("CIRC_PUMP OFF", createCirculationPumpControl(false), "f4 03 24 00 d3");

        Serial.println();

        // =========================================================================
        // KEEP-ALIVE КОМАНДЫ
        // =========================================================================
        Serial.println("🔄 KEEP-ALIVE КОМАНДЫ:");
        Serial.println("─────────────────────────────────────────────────────────");

        testCommand("KEEPALIVE PARKING", createKeepAliveParking(), "f4 04 44 21 00 95");
        testCommand("KEEPALIVE VENTILATION", createKeepAliveVentilation(), "f4 04 44 22 00 96");
        testCommand("KEEPALIVE SUPPLEMENTAL", createKeepAliveSupplemental(), "f4 04 44 23 00 97");
        testCommand("KEEPALIVE CIRC_PUMP", createKeepAliveCirculationPump(), "f4 04 44 24 00 90");
        testCommand("KEEPALIVE BOOST", createKeepAliveBoost(), "f4 04 44 25 00 91");

        Serial.println();

        // =========================================================================
        // КОМАНДЫ ЧТЕНИЯ СЕНСОРОВ (0x50)
        // =========================================================================
        Serial.println("📊 КОМАНДЫ ЧТЕНИЯ СЕНСОРОВ:");
        Serial.println("─────────────────────────────────────────────────────────");

        testCommand("SENSOR_STATUS_FLAGS", createReadSensor(SENSOR_STATUS_FLAGS), "f4 03 50 02 a5");
        testCommand("SENSOR_ON_OFF_FLAGS", createReadSensor(SENSOR_ON_OFF_FLAGS), "f4 03 50 03 a4");
        testCommand("SENSOR_FUEL_SETTINGS", createReadSensor(SENSOR_FUEL_SETTINGS), "f4 03 50 04 a3");
        testCommand("SENSOR_OPERATIONAL", createReadSensor(SENSOR_OPERATIONAL), "f4 03 50 05 a2");
        testCommand("SENSOR_OPERATING_TIMES", createReadSensor(SENSOR_OPERATING_TIMES), "f4 03 50 06 a1");
        testCommand("SENSOR_OPERATING_STATE", createReadSensor(SENSOR_OPERATING_STATE), "f4 03 50 07 a0");
        testCommand("SENSOR_BURNING_DURATION", createReadSensor(SENSOR_BURNING_DURATION), "f4 03 50 0a ad");
        testCommand("SENSOR_WORKING_DURATION", createReadSensor(SENSOR_WORKING_DURATION), "f4 03 50 0b ac"); // не удается декодировать ответ
        testCommand("SENSOR_START_COUNTERS", createReadSensor(SENSOR_START_COUNTERS), "f4 03 50 0c ab");
        testCommand("SENSOR_SUBSYSTEMS", createReadSensor(SENSOR_SUBSYSTEMS_STATUS), "f4 03 50 0f a8");
        testCommand("SENSOR_OTHER_DURATION", createReadSensor(SENSOR_OTHER_DURATION), "f4 03 50 10 b7");          // не используется
        testCommand("SENSOR_TEMP_THRESHOLDS", createReadSensor(SENSOR_TEMPERATURE_THRESHOLDS), "f4 03 50 11 b6"); // не удается декодировать ответ
        testCommand("SENSOR_VENTILATION_DUR", createReadSensor(SENSOR_VENTILATION_DURATION), "f4 03 50 12 b5");   // не используется
        testCommand("SENSOR_FUEL_PREWARMING", createReadSensor(SENSOR_FUEL_PREWARMING), "f4 03 50 13 b4");
        testCommand("SENSOR_SPARK_TRANSMISSION", createReadSensor(SENSOR_SPARK_TRANSMISSION), "f4 03 50 14 b3"); // не используется

        Serial.println();

        // =========================================================================
        // КОМАНДЫ ЧТЕНИЯ ИНФОРМАЦИИ (0x51)
        // =========================================================================
        Serial.println("ℹ️  КОМАНДЫ ЧТЕНИЯ ИНФОРМАЦИИ:");
        Serial.println("─────────────────────────────────────────────────────────");

        testCommand("INFO_DEVICE_ID", createReadInfo(INFO_DEVICE_ID), "f4 03 51 01 a7");
        testCommand("INFO_HARDWARE_VERSION", createReadInfo(INFO_HARDWARE_VERSION), "f4 03 51 02 a4"); // не используется
        testCommand("INFO_DATASET_ID", createReadInfo(INFO_DATASET_ID), "f4 03 51 03 a5");             // плохо декодируется, есть нессоответствия
        testCommand("INFO_CTRL_MFG_DATE", createReadInfo(INFO_CTRL_MFG_DATE), "f4 03 51 04 a2");
        testCommand("INFO_HEATER_MFG_DATE", createReadInfo(INFO_HEATER_MFG_DATE), "f4 03 51 05 a3");
        testCommand("INFO_UNKNOWN_06", createReadInfo(INFO_UNKNOWN_06), "f4 03 51 06 a0");
        testCommand("INFO_CUSTOMER_ID", createReadInfo(INFO_CUSTOMER_ID), "f4 03 51 07 a1");
        testCommand("INFO_SERIAL_NUMBER", createReadInfo(INFO_SERIAL_NUMBER), "f4 03 51 09 af");

        testCommand("INFO_WBUS_VERSION", createReadInfo(INFO_WBUS_VERSION), "f4 03 51 0a ac");
        testCommand("INFO_DEVICE_NAME", createReadInfo(INFO_DEVICE_NAME), "f4 03 51 0b ad");
        testCommand("INFO_WBUS_CODE", createReadInfo(INFO_WBUS_CODE), "f4 03 51 0c aa");
        testCommand("INFO_UNKNOWN_0D", createReadInfo(INFO_UNKNOWN_0D), "f4 03 51 0d ab"); // не используется

        Serial.println();

        // =========================================================================
        // КОМАНДЫ ОШИБОК (0x56)
        // =========================================================================
        Serial.println("🚨 КОМАНДЫ ОШИБОК:");
        Serial.println("─────────────────────────────────────────────────────────");

        testCommand("ERROR_READ_LIST", createReadErrors(), "f4 03 56 01 a0");
        testCommand("ERROR_CLEAR", createClearErrors(), "f4 03 56 03 a2");

        Serial.println();

        // =========================================================================
        // КОМАНДЫ ТЕСТИРОВАНИЯ КОМПОНЕНТОВ
        // =========================================================================
        Serial.println("🔧 КОМАНДЫ ТЕСТИРОВАНИЯ КОМПОНЕНТОВ:");
        Serial.println("─────────────────────────────────────────────────────────");

        testCommand("TEST_COMBUSTION_FAN (10s, 50%)",
                    createTestCombustionFan(10, 50),
                    "f4 06 45 01 0a 00 ff 43");

        testCommand("TEST_FUEL_PUMP (10s, 15Hz)",
                    createTestFuelPump(10, 15),
                    "f4 06 45 02 0a 01 2c 92");

        testCommand("TEST_GLOW_PLUG (5s, 75%)",
                    createTestGlowPlug(5, 75),
                    "f4 06 45 03 05 00 96 27");

        testCommand("TEST_CIRC_PUMP (15s, 100%)",
                    createTestCirculationPump(15, 100),
                    "f4 06 45 04 0f 00 c8 74");

        testCommand("TEST_VEHICLE_FAN (8s)",
                    createTestVehicleFan(8),
                    "f4 06 45 05 08 00 01 bb");

        testCommand("TEST_SOLENOID (12s)",
                    createTestSolenoidValve(12),
                    "f4 06 45 09 0c 00 01 b3");

        testCommand("TEST_FUEL_PREHEATING (20s, 50%)",
                    createTestFuelPreheating(20, 50),
                    "f4 06 45 0f 14 00 ff 53");

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
        minutes = constrain(minutes, 1, 59);
        return createCommandWithByte(CMD_PARK_HEAT, minutes);
    }

    static String createVentilation(uint8_t minutes = 59)
    {
        minutes = constrain(minutes, 1, 59);
        return createCommandWithByte(CMD_VENTILATE, minutes);
    }

    static String createSupplementalHeat(uint8_t minutes = 59)
    {
        minutes = constrain(minutes, 1, 59);
        return createCommandWithByte(CMD_SUPP_HEAT, minutes);
    }

    static String createBoostMode(uint8_t minutes = 59)
    {
        minutes = constrain(minutes, 1, 59);
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

    static String createFuelCirculation(uint8_t seconds)
    {
        if (seconds < 3)
            seconds = 3;

        if (seconds % 2 == 0)
        {
            seconds--;
        }

        uint8_t value = (seconds - 1) / 2;
        uint8_t data[] = {0x00, value};
        return createCommand(CMD_FUEL_CIRCULATION, 0x03, data, 2);
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