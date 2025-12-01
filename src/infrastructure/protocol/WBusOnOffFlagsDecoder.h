// src/infrastructure/protocol/WBusOnOffFlagsDecoder.h
#pragma once
#include <Arduino.h>
#include "../../domain/Entities.h"
#include "../../common/Utils.h"

class WBusOnOffFlagsDecoder
{
public:
    static OnOffFlags decode(const String &response)
    {
        OnOffFlags result;

        if (!Utils::validateASCPacketStructure(response, 0x50, 0x03, 6))
        {
            return result;
        }

        int byteCount;
        uint8_t *data = Utils::hexStringToByteArray(response, byteCount);

        uint8_t flags = data[4];
        result.combustionAirFan = (flags & 0x01) != 0;
        result.glowPlug = (flags & 0x02) != 0;
        result.fuelPump = (flags & 0x04) != 0;
        result.circulationPump = (flags & 0x08) != 0;
        result.vehicleFanRelay = (flags & 0x10) != 0;
        result.nozzleStockHeating = (flags & 0x20) != 0;
        result.flameIndicator = (flags & 0x40) != 0;

        return result;
    }

private:
};