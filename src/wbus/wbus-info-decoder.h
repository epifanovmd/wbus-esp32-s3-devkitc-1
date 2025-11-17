#ifndef WBUS_DECODER_H
#define WBUS_DECODER_H

#include <Arduino.h>

// Структуры для возврата декодированных данных
struct DecodedManufactureDate {
    String dateString;
    uint8_t day;
    uint8_t month;
    uint16_t year;
    bool isValid;
};

struct DecodedVersion {
    String versionString;
    uint8_t major;
    uint8_t minor;
    bool isValid;
};

struct DecodedWBusCode {
    String codeString;
    String supportedFunctions;
    bool isValid;
};

struct DecodedTextData {
    String text;
    bool isValid;
};

class WBusDecoder {
public:
    // Декодирование версии W-Bus
    static DecodedVersion decodeWBusVersion(const String& response);
    
    // Декодирование имени устройства
    static DecodedTextData decodeDeviceName(const String& response);
    
    // Декодирование W-Bus кода
    static DecodedWBusCode decodeWBusCode(const String& response);
    
    // Декодирование ID устройства
    static DecodedTextData decodeDeviceID(const String& response);
    
    // Декодирование даты производства нагревателя
    static DecodedManufactureDate decodeHeaterManufactureDate(const String& response);
    
    // Декодирование даты производства контроллера
    static DecodedManufactureDate decodeControllerManufactureDate(const String& response);
    
    // Декодирование Customer ID
    static DecodedTextData decodeCustomerID(const String& response);
    
    // Декодирование серийного номера
    static DecodedTextData decodeSerialNumber(const String& response);

private:
    // Вспомогательные функции
    static String analyzeWBusCodeByte(uint8_t byteNum, uint8_t byteVal);
    static String hexToString(const String& hexData);
    static String extractDataFromResponse(const String& response, const String& command, int dataLength = 0);
    static DecodedManufactureDate decodeManufactureDate(const String& response, const String& command);
};

extern WBusDecoder wBusDecoder;

#endif // WBUS_DECODER_H