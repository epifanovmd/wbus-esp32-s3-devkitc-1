#pragma once
#include <Arduino.h>
#include "../../domain/Entities.h"
#include "../../common/Utils.h"

class WBusOperatingTimesDecoder
{
public:
    static OperatingTimes decode(const String &response)
    {
        OperatingTimes result = {0, 0, 0, 0};

        if (!Utils::validateASCPacketStructure(response, 0x50, 0x07, 9))
        {
            return result;
        }

        uint8_t data[MESSAGE_BUFFER_SIZE];
        int byteCount;
        Utils::hexStringToByteArray(response, data, sizeof(data), byteCount);

        if (byteCount >= 11)
        {
            result.workingHours = (data[4] << 8) | data[5];
            result.workingMinutes = data[6];
            result.operatingHours = (data[7] << 8) | data[8];
            result.operatingMinutes = data[9];
        }

        return result;
    }
};