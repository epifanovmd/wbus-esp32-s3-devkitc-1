#ifndef WBUS_ERRORS_DECODER_H
#define WBUS_ERRORS_DECODER_H

#include <Arduino.h>
#include <map>
#include <vector>

// Структура для хранения информации об ошибке
struct WebastoError
{
    uint8_t code;
    String description;
    uint8_t counter;
    bool isActive;
    String hexCode;

    WebastoError(uint8_t errorCode = 0, uint8_t errorCounter = 0)
        : code(errorCode), counter(errorCounter), isActive(true)
    {
        hexCode = "0x";
        if (code < 0x10)
            hexCode += "0";
        hexCode += String(code, HEX);
    }
};

// Структура для хранения всех ошибок
struct ErrorCollection
{
    std::vector<WebastoError> errors;
    bool hasErrors = false;
    int errorCount = 0;
    String lastUpdate = "";

    void clear()
    {
        errors.clear();
        hasErrors = false;
        errorCount = 0;
    }

    void addError(const WebastoError &error)
    {
        errors.push_back(error);
        hasErrors = true;
        errorCount = errors.size();
    }

    bool isEmpty() const
    {
        return errors.empty();
    }
};

class WebastoErrorsDecoder
{
private:
    std::map<uint8_t, String> errorCodes;

    void hexStringToBytes(const String &hexString, uint8_t *output, int maxLength);

public:
    WebastoErrorsDecoder()
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
        errorCodes[0x8A] = "Обрыв цепи штифта накаливания или датчика пламени. ";
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

    ErrorCollection decodeErrorPacket(const String &packet);
    ErrorCollection decodeErrorList(const uint8_t *data, uint8_t dataLength);
    WebastoError decodeSingleError(uint8_t errorCode, uint8_t counter = 0);

    String getErrorDescription(uint8_t errorCode);
    bool errorExists(uint8_t errorCode);
    void decodeNakError(byte command, byte errorCode);
    String getCommandName(byte command);
    String getNakErrorDescription(byte errorCode);
    bool isNakResponse(const String &response);

    String formatErrorsForDisplay(const ErrorCollection &errorCollection);
};

extern WebastoErrorsDecoder webastoErrorsDecoder;

#endif // WBUS_ERRORS_DECODER_H