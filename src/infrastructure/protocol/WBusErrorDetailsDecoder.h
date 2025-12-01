#pragma once
#include <Arduino.h>
#include "../../domain/Entities.h"
#include "../../common/Utils.h"
#include "WBusErrorsDecoder.h"

class WBusErrorDetailsDecoder {
private:
    WBusErrorsDecoder& errorsDecoder;

public:
    WBusErrorDetailsDecoder(WBusErrorsDecoder& decoder) : errorsDecoder(decoder) {}

    ErrorDetails decode(const String& response, uint8_t errorCode) {
        ErrorDetails details;
        details.errorCode = errorCode;
        details.description = errorsDecoder.getErrorDescription(errorCode);
        
        if (response.isEmpty()) {
            return details;
        }

        String cleanRx = response;
        cleanRx.replace(" ", "");
        
        if (cleanRx.length() >= 28) { // 14 байт × 2 символа
            details.statusFlags = Utils::hexStringToByte(cleanRx.substring(10, 12));
            details.counter = Utils::hexStringToByte(cleanRx.substring(12, 14));
            details.operatingState = (Utils::hexStringToByte(cleanRx.substring(14, 16)) << 8) | 
                                    Utils::hexStringToByte(cleanRx.substring(16, 18));
            details.temperature = Utils::hexStringToByte(cleanRx.substring(18, 20));
            details.voltage = (Utils::hexStringToByte(cleanRx.substring(20, 22)) << 8) | 
                             Utils::hexStringToByte(cleanRx.substring(22, 24));
            details.operatingHours = (Utils::hexStringToByte(cleanRx.substring(24, 26)) << 8) | 
                                   Utils::hexStringToByte(cleanRx.substring(26, 28));
            details.operatingMinutes = Utils::hexStringToByte(cleanRx.substring(28, 30));
        }

        return details;
    }

    String decodeStatusFlags(uint8_t flags) {
        String result = "";
        if (flags & 0x01) result += "STORED, ";
        if (flags & 0x02) result += "ACTUAL, ";
        if (flags & 0x04) result += "PENDING, ";
        if (flags & 0x08) result += "CONFIRMED, ";
        
        if (result.length() > 0) {
            result = result.substring(0, result.length() - 2);
        }
        return result;
    }
};