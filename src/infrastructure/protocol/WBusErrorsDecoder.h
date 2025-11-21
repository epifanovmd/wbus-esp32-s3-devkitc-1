// src/infrastructure/protocol/WBusErrorsDecoder.h
#pragma once
#include <Arduino.h>
#include <map>
#include "../../domain/Entities.h"
#include "../../common/Utils.h"

class WBusErrorsDecoder {
private:
    std::map<uint8_t, String> errorCodes;

public:
    WBusErrorsDecoder() {
        initializeErrorCodes();
    }

    void initializeErrorCodes() {
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

    String getErrorDescription(uint8_t errorCode) {
        if (errorCodes.find(errorCode) != errorCodes.end()) {
            return errorCodes[errorCode];
        } else {
            return "Неизвестная ошибка, код – " + String(errorCode);
        }
    }

    bool errorExists(uint8_t errorCode) {
        return errorCodes.find(errorCode) != errorCodes.end();
    }

    void decodeNakError(uint8_t command, uint8_t errorCode) {
        String commandName = getCommandName(command);
        String errorDescription = getNakErrorDescription(errorCode);

        Serial.println();
        Serial.println("   Команда: " + commandName);
        Serial.println("   Причина: " + errorDescription);
    }

    String getCommandName(uint8_t command) {
        switch (command) {
        case 0x21: return "Parking Heat (0x21)";
        case 0x22: return "Ventilation (0x22)";
        case 0x23: return "Supplemental Heat (0x23)";
        case 0x10: return "Shutdown (0x10)";
        case 0x38: return "Diagnostic (0x38)";
        default: return "Unknown Command (0x" + String(command, HEX) + ")";
        }
    }

    String getNakErrorDescription(uint8_t errorCode) {
        switch (errorCode) {
        case 0x33: return "Невозможно выполнить в текущем состоянии";
        case 0x22: return "Неправильные параметры команды";
        case 0x11: return "Команда не поддерживается";
        case 0x44: return "Аппаратная ошибка";
        case 0x55: return "Температура вне диапазона";
        default: return "Неизвестная ошибка (0x" + String(errorCode, HEX) + ")";
        }
    }

    bool isNakResponse(const String& response) {
        return response.indexOf("4F 04 7F") == 0;
    }

    ErrorCollection decodeErrorPacket(const String& packet) {
        ErrorCollection result;
        const int MAX_PACKET_LENGTH = 32;
        uint8_t data[MAX_PACKET_LENGTH];

        hexStringToBytes(packet, data, MAX_PACKET_LENGTH);

        if (packet.length() < 8 || data[2] != 0xD6 || data[3] != 0x01) {
            return result;
        }

        uint8_t errorData[MAX_PACKET_LENGTH - 6];
        int errorDataLength = 0;
        int dataStartIndex = 4;

        for (int i = dataStartIndex; i < data[1] + 2 - 1; i++) {
            if (i < MAX_PACKET_LENGTH) {
                errorData[errorDataLength++] = data[i];
            }
        }

        return decodeErrorList(errorData, errorDataLength);
    }

    ErrorCollection decodeErrorList(const uint8_t* data, uint8_t dataLength) {
        ErrorCollection result;

        if (dataLength < 1) return result;

        uint8_t errorCount = data[0];

        if (errorCount == 0) {
            return result;
        }

        if (dataLength < 1 + errorCount * 2) {
            return result;
        }

        for (int i = 0; i < errorCount; i++) {
            uint8_t errorCode = data[1 + i * 2];
            uint8_t counter = data[2 + i * 2];
            WebastoError error = decodeSingleError(errorCode, counter);
            result.addError(error);
        }

        return result;
    }

    WebastoError decodeSingleError(uint8_t errorCode, uint8_t counter) {
        WebastoError error(errorCode, counter);
        error.description = getErrorDescription(errorCode);
        return error;
    }

    String formatErrorsForDisplay(const ErrorCollection& errorCollection) {
        String result = "";

        if (errorCollection.isEmpty()) {
            result = "✅ Ошибок не обнаружено";
            return result;
        }

        result += "📋 Найдено ошибок: " + String(errorCollection.errorCount) + "\n\n";
        
        for (size_t i = 0; i < errorCollection.errors.size(); i++) {
            const WebastoError& error = errorCollection.errors[i];
            result += String(i + 1) + ". " + error.hexCode;
            result += " (" + String(error.code, DEC) + ") - ";
            result += error.description;
            
            if (error.counter > 0) {
                result += " [Счетчик: " + String(error.counter) + "]";
            }
            
            result += "\n";
        }

        return result;
    }

private:
    void hexStringToBytes(const String& hexString, uint8_t* output, int maxLength) {
        String cleanString = hexString;
        cleanString.replace(" ", "");
        cleanString.toLowerCase();

        int length = cleanString.length();
        if (length % 2 != 0) return;

        int byteCount = length / 2;
        if (byteCount > maxLength) byteCount = maxLength;

        for (int i = 0; i < byteCount; i++) {
            String byteString = cleanString.substring(i * 2, i * 2 + 2);
            output[i] = (uint8_t)strtol(byteString.c_str(), NULL, 16);
        }
    }
};