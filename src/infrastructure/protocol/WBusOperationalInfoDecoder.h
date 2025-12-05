// src/infrastructure/protocol/WBusOperationalInfoDecoder.h
#pragma once
#include <Arduino.h>
#include "../../domain/Entities.h"
#include "../../common/Utils.h"
#include "../../common/PacketParser.h"

class WBusOperationalInfoDecoder
{
public:
    static OperationalMeasurements decode(const String &response)
    {
        OperationalMeasurements result;

        PacketParser parser;

        if (parser.parseFromString(response, WBusCommandBuilder::CMD_READ_SENSOR, PacketParser::WithIndex(WBusCommandBuilder::SENSOR_OPERATIONAL), PacketParser::WithMinLength(13)))
        {
            auto &data = parser.getBytes();

            result.temperature = data[4] - 50.0;
            result.voltage = (float)((data[5] << 8) | data[6]) / 1000.0;
            result.flameDetected = (data[7] == 0x01);
            result.heatingPower = (data[8] << 8) | data[9];
            result.flameResistance = (data[10] << 8) | data[11];
        }

        return result;
    }
};