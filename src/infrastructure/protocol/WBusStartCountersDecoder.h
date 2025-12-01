#pragma once
#include <Arduino.h>
#include "../../domain/Entities.h"
#include "../../common/Utils.h"

class WBusStartCountersDecoder {
public:
    static StartCounters decode(const String &response) {
        StartCounters result = {0, 0, 0};
        
        if (!Utils::validateASCPacketStructure(response, 0x50, 0x0C, 9)) {
            return result;
        }

        int byteCount;
        uint8_t *data = Utils::hexStringToByteArray(response, byteCount);

        if (byteCount >= 11) {
            result.shStarts = (data[4] << 8) | data[5];
            result.zhStarts = (data[6] << 8) | data[7];
            result.totalStarts = (data[8] << 8) | data[9];
        }

        return result;
    }
};