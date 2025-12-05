#pragma once
#include <Arduino.h>
#include "../../domain/Entities.h"
#include "../../common/Utils.h"

class WBusOperatingTimesDecoder
{
public:
    static OperatingTimes decode(const String &response)
    {
        OperatingTimes result = {0, 0, 0, 0, 0};

        PacketParser parser;

        if (parser.parseFromString(response, WBusCommandBuilder::CMD_READ_SENSOR, PacketParser::WithIndex(WBusCommandBuilder::SENSOR_OPERATING_TIMES), PacketParser::WithMinLength(13)))
        {
            auto &data = parser.getBytes();

            result.workingHours = (data[4] << 8) | data[5];
            result.workingMinutes = data[6];
            result.operatingHours = (data[7] << 8) | data[8];
            result.operatingMinutes = data[9];
            result.startCounter = ((data[10] << 8) | data[11]);
        }

        return result;
    }
};