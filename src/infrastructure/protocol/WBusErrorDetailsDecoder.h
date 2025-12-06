#pragma once
#include <Arduino.h>
#include "../../domain/Entities.h"
#include "../../common/Utils.h"
#include "WBusErrorsDecoder.h"

class WBusErrorDetailsDecoder
{
private:
    float decodeTemperature(uint8_t tempByte)
    {
        return static_cast<float>(static_cast<int8_t>(tempByte) - 50);
    }

    String decodeStatusFlags(uint8_t statusByte)
    {
        String flags;
        bool first = true;

        if (statusByte & 0x01)
        {
            flags += "Сохранена в памяти";
            first = false;
        }

        if (statusByte & 0x02)
        {
            if (!first)
                flags += ", ";
            flags += "Активна";
        }

        if (flags.isEmpty())
        {
            flags = "Статус неизвестен (0x" + String(statusByte, HEX) + ")";
        }

        return flags;
    }

public:
    WBusErrorDetailsDecoder() {}

    ErrorDetails decode(const String &response, uint8_t errorCode)
    {
        ErrorDetails details;

        PacketParser parser;

        if (parser.parseFromString(response, WBusCommandBuilder::CMD_READ_ERRORS, PacketParser::WithIndex(WBusCommandBuilder::ERROR_READ_DETAILS), PacketParser::WithMinLength(16)))
        {
            auto &data = parser.getBytes();

            // Проверка кода ошибки в ответе
            if (data[4] != errorCode)
            {
                return details;
            }

            // Извлекаем данные (начиная с байта 4)
            int dataIndex = 4; // байт с кодом ошибки

            // Код ошибки (байт 4)
            details.errorCode = Utils::formatHexString(data[dataIndex++]);

            // 1. Статус ошибки
            uint8_t statusByte = data[dataIndex++];
            details.status = decodeStatusFlags(statusByte);

            // 2. Счетчик срабатываний (в протоколе: counter-1)
            uint8_t counterRaw = data[dataIndex++];
            details.counter = counterRaw + 1; // Преобразуем counter-1 в фактическое количество

            // 3. Состояние работы (2 байта: код состояния и номер состояния)
            details.stateCode = Utils::formatHexString(data[dataIndex++]); // байт 7 - код состояния
            uint8_t stateNumber = data[dataIndex++];                       // байт 8 - номер состояния
            details.stateName = WBusOperatingStateDecoder::getStateName(stateNumber);

            // 4. Температура (смещение +50°C)
            uint8_t tempRaw = data[dataIndex++]; // байт 9
            details.temperature = static_cast<float>(static_cast<int8_t>(tempRaw) - 50);

            // 5. Напряжение (2 байта, big-endian, в милливольтах)
            // (high << 8) | low
            uint8_t voltageHigh = data[dataIndex++]; // байт 10
            uint8_t voltageLow = data[dataIndex++];  // байт 11
            details.voltage = (static_cast<float>((static_cast<uint16_t>(voltageHigh) << 8) | voltageLow)) / 1000.0f;

            // 6. Время работы (2 байта часы + 1 байт минуты, big-endian)
            uint8_t hoursHigh = data[dataIndex++];                                       // байт 12
            uint8_t hoursLow = data[dataIndex++];                                        // байт 13
            details.operatingHours = (static_cast<uint32_t>(hoursHigh) << 8) | hoursLow; // big-endian
            details.operatingMinutes = data[dataIndex++];                                // байт 14
        }

        return details;
    }
};