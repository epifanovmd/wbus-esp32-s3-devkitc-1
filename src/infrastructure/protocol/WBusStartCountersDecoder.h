#pragma once
#include <Arduino.h>
#include "../../domain/Entities.h"
#include "../../common/Utils.h"

class WBusStartCountersDecoder
{
public:
    static StartCounters decode(const String &response)
    {
        StartCounters result = {0, 0, 0};

        PacketParser parser;

        if (parser.parseFromString(response, WBusCommandBuilder::CMD_READ_SENSOR, PacketParser::WithIndex(WBusCommandBuilder::SENSOR_START_COUNTERS), PacketParser::WithMinLength(11)))
        {
            auto &data = parser.getBytes();

            result.shStarts = (data[4] << 8) | data[5];
            result.zhStarts = (data[6] << 8) | data[7];
            result.totalStarts = (data[8] << 8) | data[9];
        }

        return result;
    }
};
