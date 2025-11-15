#ifndef WBUS_ERROR__H
#define WBUS_ERROR__H

#include <Arduino.h>
#include <functional>
#include "wbus/wbus-queue.h"
#include "wbus/wbus.constants.h"
#include "wbus-error-codes.h"

class WebastoError
{
private:
    WebastoErrorCodes errorCodes;

public:
    // Функция для преобразования HEX строки в массив байт
    void hexStringToBytes(const String &hexString, uint8_t *output, int maxLength)
    {
        String cleanString = hexString;
        cleanString.replace(" ", "");
        cleanString.toLowerCase();

        int length = cleanString.length();
        if (length % 2 != 0)
            return;

        int byteCount = length / 2;
        if (byteCount > maxLength)
            byteCount = maxLength;

        for (int i = 0; i < byteCount; i++)
        {
            String byteString = cleanString.substring(i * 2, i * 2 + 2);
            output[i] = (uint8_t)strtol(byteString.c_str(), NULL, 16);
        }
    }

    // Функция декодирования ошибки из полного пакета W-Bus
    String decodeErrorPacket(const String &packet)
    {
        const int MAX_PACKET_LENGTH = 32;
        uint8_t data[MAX_PACKET_LENGTH];

        // Преобразуем строку в массив байт
        hexStringToBytes(packet, data, MAX_PACKET_LENGTH);

        String result = "";

        // Проверяем базовую структуру пакета
        if (packet.length() < 8 || data[2] != 0xD6 || data[3] != 0x01)
        {
            return "Неверный формат пакета";
        }

        // Извлекаем данные ошибок
        uint8_t errorData[MAX_PACKET_LENGTH - 6];
        int errorDataLength = 0;
        // Ищем начало данных ошибок (после заголовка, длины, команды, индекса)
        // Формат: 4f 0a d6 01 [данные_ошибок] crc
        int dataStartIndex = 4; // Пропускаем 4 байта заголовка

        // Копируем данные ошибок (исключая CRC)
        for (int i = dataStartIndex; i < data[1] + 2 - 1; i++)
        {
            if (i < MAX_PACKET_LENGTH)
            {
                errorData[errorDataLength++] = data[i];
            }
        }

        // Декодируем список ошибок
        return decodeErrorList(errorData, errorDataLength);
    }

    // Функция декодирования списка ошибок из массива данных
    String decodeErrorList(const uint8_t *data, uint8_t dataLength)
    {
        String result = "";

        if (dataLength < 1)
            return "Нет данных об ошибках";

        uint8_t errorCount = data[0];

        if (errorCount == 0)
        {
            return "Ошибок не обнаружено";
        }

        if (dataLength < 1 + errorCount * 2)
        {
            return "Недостаточно данных для декодирования";
        }

        for (int i = 0; i < errorCount; i++)
        {
            uint8_t errorCode = data[1 + i * 2];
            uint8_t counter = data[2 + i * 2];

            result += String(i + 1);
            result += ". ";
            result += decodeSingleError(errorCode, counter);
            result += "\n";
        }

        return result;
    }

    // Функция декодирования одной ошибки
    String decodeSingleError(uint8_t errorCode, uint8_t counter = 0)
    {
        String result = "0x";
        if (errorCode < 0x10)
            result += "0";
        result += String(errorCode, HEX);
        result += " (";
        result += String(errorCode, DEC);
        result += ") - ";

        result += errorCodes.getErrorDescription(errorCode);

        if (counter > 0)
        {
            result += " [Счетчик: ";
            result += String(counter);
            result += "]";
        }

        return result;
    }

    // Статический метод callback
    static void checkCallback(bool status, String tx, String rx)
    {
        WebastoError decoder;
        String result = decoder.decodeErrorPacket(rx);
        Serial.println();
        Serial.print("=== ОШИБКИ WEBASTO ===");
        Serial.println();
        Serial.print(result);
        Serial.println();
    }

    void check()
    {
        wbusQueue.add(CMD_READ_ERRORS_LIST, checkCallback);
    }

    void clear()
    {
        wbusQueue.add(CMD_CLEAR_ERRORS);
    }
};

extern WebastoError webastoError;

#endif // WBUS_ERROR__H