#pragma once
#include <Arduino.h>
#include "../../domain/Entities.h"
#include "../../common/Utils.h"

class WBusBurningDurationDecoder
{
public:
    static BurningDuration decode(const String &response)
    {
        BurningDuration result;

        if (!Utils::validateASCPacketStructure(response, 0x50, 0x0A, 29))
        {
            return result;
        }

        uint8_t data[MESSAGE_BUFFER_SIZE];
        int byteCount;
        Utils::hexStringToByteArray(response, data, sizeof(data), byteCount);

        // SH 0-33% (байты 4-6)
        result.shLow = decodePowerLevel(data, 4);
        // SH 34-66% (байты 7-9)
        result.shMedium = decodePowerLevel(data, 7);
        // SH 67-100% (байты 10-12)
        result.shHigh = decodePowerLevel(data, 10);
        // SH >100% (байты 13-15)
        result.shBoost = decodePowerLevel(data, 13);

        // ZH 0-33% (байты 16-18)
        result.zhLow = decodePowerLevel(data, 16);
        // ZH 34-66% (байты 19-21)
        result.zhMedium = decodePowerLevel(data, 19);
        // ZH 67-100% (байты 22-24)
        result.zhHigh = decodePowerLevel(data, 22);
        // ZH >100% (байты 25-27)
        result.zhBoost = decodePowerLevel(data, 25);

        return result;
    }

private:
    static PowerLevelStats decodePowerLevel(uint8_t *data, int startIndex)
    {
        PowerLevelStats stats;
        if (startIndex + 2 < 29)
        {
            stats.hours = (data[startIndex] << 8) | data[startIndex + 1];
            stats.minutes = data[startIndex + 2];
        }
        return stats;
    }
};