#include "wbus-errors-decoder.h"

WebastoErrorsDecoder webastoErrorsDecoder;

String WebastoErrorsDecoder::getErrorDescription(uint8_t errorCode)
{
    if (errorCodes.find(errorCode) != errorCodes.end())
    {
        return errorCodes[errorCode];
    }
    else
    {
        return "Неизвестная ошибка, код – " + errorCode;
    }
}

bool WebastoErrorsDecoder::errorExists(uint8_t errorCode)
{
    return errorCodes.find(errorCode) != errorCodes.end();
}

void WebastoErrorsDecoder::decodeNakError(byte command, byte errorCode)
{
    String commandName = getCommandName(command);
    String errorDescription = getNakErrorDescription(errorCode);

    Serial.println();
    Serial.println("   Команда: " + commandName);
    Serial.println("   Причина: " + errorDescription);
}

String WebastoErrorsDecoder::getCommandName(byte command)
{
    switch (command)
    {
    case 0x21:
        return "Parking Heat (0x21)";
    case 0x22:
        return "Ventilation (0x22)";
    case 0x23:
        return "Supplemental Heat (0x23)";
    case 0x10:
        return "Shutdown (0x10)";
    case 0x38:
        return "Diagnostic (0x38)";
    default:
        return "Unknown Command (0x" + String(command, HEX) + ")";
    }
}

String WebastoErrorsDecoder::getNakErrorDescription(byte errorCode)
{
    switch (errorCode)
    {
    case 0x33:
        return "Невозможно выполнить в текущем состоянии";
    case 0x22:
        return "Неправильные параметры команды";
    case 0x11:
        return "Команда не поддерживается";
    case 0x44:
        return "Аппаратная ошибка";
    case 0x55:
        return "Температура вне диапазона";
    default:
        return "Неизвестная ошибка (0x" + String(errorCode, HEX) + ")";
    }
}

bool WebastoErrorsDecoder::isNakResponse(const String &response)
{
    // Формат: 4F 04 7F [command] [error_code] [crc]
    return response.indexOf("4F 04 7F") == 0;
}

// Функция для преобразования HEX строки в массив байт
void WebastoErrorsDecoder::hexStringToBytes(const String &hexString, uint8_t *output, int maxLength)
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

ErrorCollection WebastoErrorsDecoder::decodeErrorPacket(const String &packet)
{
    ErrorCollection result;
    const int MAX_PACKET_LENGTH = 32;
    uint8_t data[MAX_PACKET_LENGTH];

    hexStringToBytes(packet, data, MAX_PACKET_LENGTH);

    // Проверяем базовую структуру пакета
    if (packet.length() < 8 || data[2] != 0xD6 || data[3] != 0x01)
    {
        return result;
    }

    // Извлекаем данные ошибок
    uint8_t errorData[MAX_PACKET_LENGTH - 6];
    int errorDataLength = 0;
    int dataStartIndex = 4;

    for (int i = dataStartIndex; i < data[1] + 2 - 1; i++)
    {
        if (i < MAX_PACKET_LENGTH)
        {
            errorData[errorDataLength++] = data[i];
        }
    }

    return decodeErrorList(errorData, errorDataLength);
}

ErrorCollection WebastoErrorsDecoder::decodeErrorList(const uint8_t *data, uint8_t dataLength)
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

WebastoError WebastoErrorsDecoder::decodeSingleError(uint8_t errorCode, uint8_t counter)
{
    WebastoError error(errorCode, counter);
    error.description = getErrorDescription(errorCode);
    return error;
}


String WebastoErrorsDecoder::formatErrorsForDisplay(const ErrorCollection& errorCollection)
{
    String result = "";

    if (errorCollection.isEmpty())
    {
        result = "✅ Ошибок не обнаружено";
        return result;
    }

    result += "📋 Найдено ошибок: " + String(errorCollection.errorCount) + "\n\n";
    
    for (size_t i = 0; i < errorCollection.errors.size(); i++)
    {
        const WebastoError& error = errorCollection.errors[i];
        result += String(i + 1) + ". " + error.hexCode;
        result += " (" + String(error.code, DEC) + ") - ";
        result += error.description;
        
        if (error.counter > 0)
        {
            result += " [Счетчик: " + String(error.counter) + "]";
        }
        
        result += "\n";
    }

    return result;
}
