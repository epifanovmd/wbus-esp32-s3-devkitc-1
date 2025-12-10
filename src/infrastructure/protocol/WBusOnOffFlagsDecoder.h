// src/infrastructure/protocol/WBusOnOffFlagsDecoder.h
#pragma once
#include <Arduino.h>
#include "../../domain/Entities.h"
#include "../../common/Utils.h"
#include "../../common/PacketParser.h"

class WBusOnOffFlagsDecoder
{
public:
    static OnOffFlags decode(const String &response)
    {
        OnOffFlags result;

        PacketParser parser;

        if (parser.parseFromString(response, WBusCommandBuilder::CMD_READ_SENSOR, PacketParser::WithIndex(WBusCommandBuilder::SENSOR_ON_OFF_FLAGS), PacketParser::WithMinLength(6)))
        {
            auto &data = parser.getBytes();

            uint8_t flags = data[4];
            result.combustionAirFan = (flags & 0x01) != 0;
            result.glowPlug = (flags & 0x02) != 0;
            result.fuelPump = (flags & 0x04) != 0;
            result.circulationPump = (flags & 0x08) != 0;
            result.vehicleFanRelay = (flags & 0x10) != 0;
            result.fuelPreheating = (flags & 0x20) != 0;
            result.flameIndicator = (flags & 0x40) != 0;
        }

        return result;
    }
};