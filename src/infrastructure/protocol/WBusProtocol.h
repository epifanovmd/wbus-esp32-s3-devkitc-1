// src/infrastructure/protocol/WBusProtocol.h
#pragma once
#include <Arduino.h>
#include "../../common/Constants.h"
#include "../../common/Utils.h"

// =================================================================================
// Структура пакета:
// [ Заголовок | Длина | Команда | Данные ... | Контрольная сумма ]
// =================================================================================

// Структуры из оригинального wbus-sender.h
struct WBusPacket
{
    byte data[MESSAGE_BUFFER_SIZE];
    unsigned long byteCount;
};

class WBusProtocol
{
public:
    static WBusPacket parseHexStringToPacket(String input)
    {
        WBusPacket packet;
        packet.byteCount = 0;

        input.trim();
        input.toUpperCase();

        int startIndex = 0;
        int spaceIndex = input.indexOf(' ');

        while (spaceIndex != -1 && packet.byteCount < MESSAGE_BUFFER_SIZE)
        {
            String byteStr = input.substring(startIndex, spaceIndex);
            byteStr.trim();

            if (byteStr.length() > 0 && Utils::isHexString(byteStr))
            {
                packet.data[packet.byteCount++] = (byte)strtol(byteStr.c_str(), NULL, 16);
            }

            startIndex = spaceIndex + 1;
            spaceIndex = input.indexOf(' ', startIndex);
        }

        // Последний байт
        if (startIndex < input.length() && packet.byteCount < MESSAGE_BUFFER_SIZE)
        {
            String byteStr = input.substring(startIndex);
            byteStr.trim();

            if (byteStr.length() > 0 && Utils::isHexString(byteStr))
            {
                packet.data[packet.byteCount++] = (byte)strtol(byteStr.c_str(), NULL, 16);
            }
        }

        return packet;
    }

    static bool validateWbusPacket(WBusPacket packet)
    {
        // Проверка минимальной длины
        if (packet.byteCount < 3)
        {
            Serial.println("❌ Слишком короткий пакет (минимум 3 байта)");
            return false;
        }

        // Проверка заголовка
        byte header = packet.data[0];
        if (header != TXHEADER && header != RXHEADER)
        {
            Serial.println("❌ Неверный заголовок: " + String(header, HEX));
            return false;
        }

        // Проверка длины пакета
        byte declaredLength = packet.data[1];
        if (packet.byteCount != declaredLength + 2)
        {
            Serial.println("❌ Несоответствие длины: объявлено " + String(declaredLength) +
                           ", фактически " + String(packet.byteCount - 2));
            return false;
        }

        // Проверка контрольной суммы
        byte calculatedChecksum = 0;
        for (int i = 0; i < packet.byteCount - 1; i++)
        {
            calculatedChecksum ^= packet.data[i];
        }

        byte receivedChecksum = packet.data[packet.byteCount - 1];

        if (calculatedChecksum != receivedChecksum)
        {
            Serial.println("❌ Контрольная сумма неверна!");
            Serial.print("   Ожидалось: ");
            Utils::printHex(calculatedChecksum, false);
            Serial.print(", получено: ");
            Utils::printHex(receivedChecksum, true);
            return false;
        }

        return true;
    }

    static String createParkHeatCommand(uint8_t minutes = 59)
    {
        minutes = constrain(minutes, 1, 255);
        byte data[] = {0xF4, 0x03, 0x21, minutes};
        byte checksum = Utils::calculateChecksum(data, 4);
        return "F4 03 21 " + Utils::byteToHexString(minutes) + " " + Utils::byteToHexString(checksum);
    }

    static String createVentilateCommand(uint8_t minutes = 59)
    {
        minutes = constrain(minutes, 1, 255);
        byte data[] = {0xF4, 0x03, 0x22, minutes};
        byte checksum = Utils::calculateChecksum(data, 4);
        return "F4 03 22 " + Utils::byteToHexString(minutes) + " " + Utils::byteToHexString(checksum);
    }

    static String createSuppHeatCommand(uint8_t minutes = 59)
    {
        minutes = constrain(minutes, 1, 255);
        byte data[] = {0xF4, 0x03, 0x23, minutes};
        byte checksum = Utils::calculateChecksum(data, 4);
        return "F4 03 23 " + Utils::byteToHexString(minutes) + " " + Utils::byteToHexString(checksum);
    }

    static String createCircPumpCommand(bool enable)
    {
        uint8_t dataByte = enable ? 0x01 : 0x00;
        byte data[] = {0xF4, 0x03, 0x24, dataByte};
        byte checksum = Utils::calculateChecksum(data, 4);
        return "F4 03 24 " + Utils::byteToHexString(dataByte) + " " + Utils::byteToHexString(checksum);
    }

    static String createBoostCommand(uint8_t minutes = 59)
    {
        minutes = constrain(minutes, 1, 255);
        byte data[] = {0xF4, 0x03, 0x25, minutes};
        byte checksum = Utils::calculateChecksum(data, 4);
        return "F4 03 25 " + Utils::byteToHexString(minutes) + " " + Utils::byteToHexString(checksum);
    }

    // Команды тестирования компонентов
    static String createTestCAFCommand(uint8_t seconds, uint8_t powerPercent)
    {
        uint16_t magnitude = powerPercent * 2; // 1% = 2 единицы (50% = 0x64)
        return createTestCommand(0x01, seconds, magnitude);
    }

    static String createTestFuelPumpCommand(uint8_t seconds, uint8_t frequencyHz)
    {
        uint16_t magnitude = frequencyHz * 20; // 1 Гц = 20 единиц (10 Гц = 0xC8)
        return createTestCommand(0x02, seconds, magnitude);
    }

    static String createTestGlowPlugCommand(uint8_t seconds, uint8_t powerPercent)
    {
        uint16_t magnitude = powerPercent * 2; // 1% = 2 единицы (75% = 0x96)
        return createTestCommand(0x03, seconds, magnitude);
    }

    static String createTestCircPumpCommand(uint8_t seconds, uint8_t powerPercent)
    {
        uint16_t magnitude = powerPercent * 2; // 1% = 2 единицы (100% = 0xC8)
        return createTestCommand(0x04, seconds, magnitude);
    }

    static String createTestVehicleFanCommand(uint8_t seconds)
    {
        return createTestCommand(0x05, seconds, 0x0001); // всегда 0x0001 для реле
    }

    static String createTestSolenoidCommand(uint8_t seconds)
    {
        return createTestCommand(0x09, seconds, 0x0001); // всегда 0x0001 для соленоида
    }

    static String createTestFuelPreheatCommand(uint8_t seconds, uint8_t powerPercent)
    {
        uint16_t magnitude = powerPercent * 2; // 1% = 2 единицы (50% = 0x64)
        return createTestCommand(0x0F, seconds, magnitude);
    }
    
    // =================================================================================
    // БАЗОВЫЕ КОМАНДЫ УПРАВЛЕНИЯ (теперь статические)
    // =================================================================================

    static const String CMD_SHUTDOWN;
    static const String CMD_PARK_HEAT;
    static const String CMD_VENTILATE;
    static const String CMD_SUPP_HEAT;
    static const String CMD_CIRC_PUMP_CTRL;
    static const String CMD_BOOST_MODE;

    // =================================================================================
    // КОМАНДЫ ДИАГНОСТИКИ И СЕРВИСА
    // =================================================================================

    static const String CMD_DIAGNOSTIC;
    static const String CMD_KEEPALIVE_PARKING;
    static const String CMD_KEEPALIVE_VENT;
    static const String CMD_KEEPALIVE_SUPP_HEAT;
    static const String CMD_KEEPALIVE_CIRC_PUMP;
    static const String CMD_KEEPALIVE_BOOST;

    // =================================================================================
    // КОМАНДЫ ЧТЕНИЯ ДАННЫХ - СЕНСОРЫ (0x50)
    // =================================================================================

    static const String CMD_READ_SENSOR_STATUS_FLAGS;
    static const String CMD_READ_SENSOR_ON_OFF_FLAGS;
    static const String CMD_READ_SENSOR_FUEL_SETTINGS;
    static const String CMD_READ_SENSOR_OPERATIONAL;
    static const String CMD_READ_SENSOR_OPERATING_STATE;
    static const String CMD_READ_SENSOR_SUBSYSTEMS_STATUS;

    // =================================================================================
    // КОМАНДЫ ЧТЕНИЯ ИНФОРМАЦИИ (0x51)
    // =================================================================================

    static const String CMD_READ_INFO_DEVICE_ID;
    static const String CMD_READ_INFO_CTRL_MFG_DATE;
    static const String CMD_READ_INFO_HEATER_MFG_DATE;
    static const String CMD_READ_INFO_CUSTOMER_ID;
    static const String CMD_READ_INFO_SERIAL_NUMBER;
    static const String CMD_READ_INFO_WBUS_VERSION;
    static const String CMD_READ_INFO_DEVICE_NAME;
    static const String CMD_READ_INFO_WBUS_CODE;

    // =================================================================================
    // КОМАНДЫ ОШИБОК (0x56)
    // =================================================================================

    static const String CMD_READ_ERRORS_LIST;
    static const String CMD_CLEAR_ERRORS;

private:
    static String createTestCommand(uint8_t component, uint8_t seconds, uint16_t magnitude)
    {
        byte data[] = {
            0xF4, 0x06, 0x45, component,
            seconds,
            (byte)(magnitude >> 8),
            (byte)(magnitude & 0xFF)};
        byte checksum = Utils::calculateChecksum(data, 7);

        return "F4 06 45 " +
               Utils::byteToHexString(component) + " " +
               Utils::byteToHexString(seconds) + " " +
               Utils::byteToHexString(magnitude >> 8) + " " +
               Utils::byteToHexString(magnitude & 0xFF) + " " +
               Utils::byteToHexString(checksum);
    }
};