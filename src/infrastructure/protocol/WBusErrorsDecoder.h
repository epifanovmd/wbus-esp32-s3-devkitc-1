// src/infrastructure/protocol/WBusErrorsDecoder.h
#pragma once
#include <Arduino.h>
#include <map>
#include "../../domain/Entities.h"
#include "../../common/Utils.h"

class WBusErrorsDecoder
{
private:
    std::map<uint8_t, String> errorCodes;

public:
    WBusErrorsDecoder()
    {
        initializeErrorCodes();
    }

    void initializeErrorCodes()
    {
        // Базовые ошибки системы
        errorCodes[0x00] = "Нет ошибок";
        errorCodes[0x01] = "Ошибка блока управления";
        errorCodes[0x02] = "Нет запуска";
        errorCodes[0x04] = "Повышенное напряжение";
        errorCodes[0x05] = "Преждевременное распознавание пламени";

        // Ошибки компонентов (короткое замыкание)
        errorCodes[0x08] = "Короткое замыкание насоса-дозатора";
        errorCodes[0x0B] = "Короткое замыкание циркуляционного насоса";
        errorCodes[0x10] = "Короткое замыкание в клапане переключения ОЖ";
        errorCodes[0x13] = "Короткое замыкание в штатном вентиляторе автомобиля";
        errorCodes[0x19] = "Короткое замыкание в цепи штифта накала";
        errorCodes[0x1B] = "Короткое замыкание датчика перегрева";

        // Ошибки программирования и конфигурации
        errorCodes[0x11] = "Неправильно запрограммированный блок управления";
        errorCodes[0x3F] = "Загружено неправильное программное обеспечение";
        errorCodes[0x81] = "Ошибка контрольной суммы EOL";

        // Ошибки механических компонентов
        errorCodes[0x15] = "Защита от блокировки мотора нагнетателя";
        errorCodes[0x22] = "При старте не было достигнуто контрольное сопротивление";
        errorCodes[0x2D] = "Неисправность в цепи нагнетателя";
        errorCodes[0x2E] = "Неисправность в цепи штифта накала";
        errorCodes[0x2F] = "Обрыв пламени";

        // Ошибки температуры
        errorCodes[0x37] = "Слишком высокая температура ОЖ при первом вводе в эксплуатацию";
        errorCodes[0x86] = "Слишком высокая температура ОЖ без процесса горения";

        // Ошибки запуска
        errorCodes[0x38] = "Первая попытка запуска неудачная";
        errorCodes[0x39] = "Первая попытка запуска неудачная - нет повторного запуска";
        errorCodes[0x82] = "Нет запуска в тестовом режиме";
        errorCodes[0x83] = "Обрыв пламени (FAZ)";

        // Ошибки напряжения
        errorCodes[0x84] = "Пониженное напряжение";
        errorCodes[0x4C] = "Высокое напряжение при защите компонентов";
        errorCodes[0x9C] = "Интеллектуальное отключение при пониженном напряжении";

        // Ошибки обрыва цепей
        errorCodes[0x88] = "Обрыв насоса-дозатора";
        errorCodes[0x89] = "Обрыв нагнетателя";
        errorCodes[0x8B] = "Обрыв циркуляционного насоса";
        errorCodes[0x8A] = "Обрыв цепи штифта накаливания или датчика пламени";
        errorCodes[0x90] = "Обрыв в клапане переключения ОЖ";
        errorCodes[0x94] = "Обрыв температурного датчика";
        errorCodes[0x99] = "Обрыв штифта накала";
        errorCodes[0xAB] = "Обрыв датчика перегрева";

        // Ошибки коммуникации
        errorCodes[0x92] = "Ошибка в обработке команд";
        errorCodes[0xAA] = "Неудачная отправка команд в шину W-Bus";

        // Внутренние ошибки блока управления
        errorCodes[0x3C] = "Внутренняя ошибка блока управления 60";
        errorCodes[0x3D] = "Внутренняя ошибка блока управления 61";
        errorCodes[0x3E] = "Внутренняя ошибка блока управления 62";

        // Прочие ошибки
        errorCodes[0x5A] = "Короткое замыкание в шине W-Bus / LIN-Bus";
        errorCodes[0x62] = "Переполнение значение таймера DP_max";
        errorCodes[0x87] = "Постоянная блокировка подогревателя";
    }

    String getErrorDescription(uint8_t errorCode)
    {
        if (errorCodes.find(errorCode) != errorCodes.end())
        {
            return errorCodes[errorCode];
        }
        else
        {
            return "Неизвестная ошибка, код – " + String(errorCode);
        }
    }

    bool errorExists(uint8_t errorCode)
    {
        return errorCodes.find(errorCode) != errorCodes.end();
    }

    ErrorCollection decodeErrorPacket(const String &packet)
    {
        ErrorCollection result;

        uint8_t data[MESSAGE_BUFFER_SIZE];
        int byteCount;
        Utils::hexStringToByteArray(packet, data, sizeof(data), byteCount);

        if (packet.length() < 8 || data[2] != 0xD6 || data[3] != 0x01)
        {
            return result;
        }

        return decodeErrorList(data, byteCount);
    }

    ErrorCollection decodeErrorList(const uint8_t *data, uint8_t dataLength)
    {
        ErrorCollection result;

        if (dataLength < 1)
            return result;

        uint8_t errorCount = data[0];

        if (errorCount == 0)
        {
            return result;
        }

        if (dataLength < 1 + errorCount * 2)
        {
            return result;
        }

        for (int i = 0; i < errorCount; i++)
        {
            uint8_t errorCode = data[1 + i * 2];
            uint8_t counter = data[2 + i * 2];
            WebastoError error = decodeSingleError(errorCode, counter);
            result.addError(error);
        }

        return result;
    }

    WebastoError decodeSingleError(uint8_t errorCode, uint8_t counter)
    {
        WebastoError error(errorCode, counter);
        error.description = getErrorDescription(errorCode);
        return error;
    }
};