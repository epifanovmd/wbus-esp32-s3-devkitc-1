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
            flags += "Активная ошибка";
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

        if (parser.parseFromString(response, WBusCommandBuilder::CMD_READ_SENSOR, PacketParser::WithIndex(WBusCommandBuilder::ERROR_READ_DETAILS), PacketParser::WithMinLength(16)))
        {
            auto &data = parser.getBytes();

            // Проверка кода ошибки в ответе
            if (data[4] != errorCode)
            {
                Serial.println("Код ошибки не совпадает");
                return details;
            }

            // Извлекаем данные (начиная с байта 4)
            int dataIndex = 4; // байт с кодом ошибки

            // Код ошибки уже проверен (байт 4)
            dataIndex++; // переходим к байту 5

            // 1. Статус ошибки
            uint8_t statusByte = data[dataIndex++];
            details.status = decodeStatusFlags(statusByte);

            // 2. Счетчик срабатываний (в протоколе: counter-1)
            uint8_t counterRaw = data[dataIndex++];
            details.counter = counterRaw + 1; // Преобразуем counter-1 в фактическое количество

            // 3. Состояние работы (2 байта: код состояния и номер состояния)
            // ВАЖНО: У вас пропущен второй байт состояния!
            // В пакете: байт 7 = код состояния, байт 8 = номер состояния

            details.stateCode = "0x" + Utils::byteToHexString(data[dataIndex++]); // байт 7 - код состояния (0x00)
            uint8_t stateNumber = data[dataIndex++];                              // байт 8 - номер состояния (0x54) - ВЫ ПРОПУСТИЛИ ЭТОТ БАЙТ!

            // Для обратной совместимости, stateName должен декодироваться по stateNumber
            // stateNumber = 0x54 = 84 = "Heater interlock permanent"
            details.stateName = WBusOperatingStateDecoder::getStateName(stateNumber); // Должен принимать stateNumber!

            // 4. Температура (смещение +50°C)
            uint8_t tempRaw = data[dataIndex++]; // байт 9
            details.temperature = static_cast<float>(static_cast<int8_t>(tempRaw) - 50);

            // 5. Напряжение (2 байта, big-endian, в милливольтах)
            // ВАЖНО: Неправильный порядок байт! Вы используете little-endian
            // Правильно: (high << 8) | low
            // Неправильно: (low << 8) | high
            uint8_t voltageHigh = data[dataIndex++]; // байт 10
            uint8_t voltageLow = data[dataIndex++];  // байт 11
            details.voltage = (static_cast<float>((static_cast<uint16_t>(voltageHigh) << 8) | voltageLow)) / 1000.0f;

            // 6. Время работы (2 байта часы + 1 байт минуты, big-endian)
            // ВАЖНО: Та же ошибка с порядком байт!
            uint8_t hoursHigh = data[dataIndex++];                                       // байт 12
            uint8_t hoursLow = data[dataIndex++];                                        // байт 13
            details.operatingHours = (static_cast<uint32_t>(hoursHigh) << 8) | hoursLow; // big-endian
            details.operatingMinutes = data[dataIndex++];                                // байт 14
        }

        return details;
    }
};