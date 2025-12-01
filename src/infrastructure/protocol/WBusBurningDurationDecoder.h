#pragma once
#include <Arduino.h>
#include "../../domain/Entities.h"
#include "../../common/Utils.h"

class WBusBurningDurationDecoder {
public:
    static BurningDuration decode(const String &response) {
        BurningDuration result;
        
        if (!Utils::validateASCPacketStructure(response, 0x50, 0x0A, 27)) {
            return result;
        }

        uint8_t data[MESSAGE_BUFFER_SIZE];
        int byteCount;
        Utils::hexStringToByteArray(response, data, sizeof(data), byteCount);

        if (byteCount >= 29) { // 4 байта заголовок + 25 байт данных
            // SH 0-33% (байты 4-9)
            result.shLow = decodePowerLevel(data, 4);
            // SH 34-66% (байты 10-15)
            result.shMedium = decodePowerLevel(data, 10);
            // SH 67-100% (байты 16-21)
            result.shHigh = decodePowerLevel(data, 16);
            // SH >100% (байты 22-27)
            result.shBoost = decodePowerLevel(data, 22);
            
            // ZH 0-33% (байты 28-33)
            result.zhLow = decodePowerLevel(data, 28);
            // ZH 34-66% (байты 34-39)
            result.zhMedium = decodePowerLevel(data, 34);
            // ZH 67-100% (байты 40-45)
            result.zhHigh = decodePowerLevel(data, 40);
            // ZH >100% (байты 46-51)
            result.zhBoost = decodePowerLevel(data, 46);
        }

        return result;
    }

private:
    static PowerLevelStats decodePowerLevel(uint8_t* data, int startIndex) {
        PowerLevelStats stats;
        if (startIndex + 5 < 52) { // Проверяем границы
            stats.hours = (data[startIndex] << 8) | data[startIndex + 1];
            stats.minutes = data[startIndex + 2];
            // Байты startIndex+3, +4, +5 - возможно резерв или дополнительные данные
        }
        return stats;
    }
};