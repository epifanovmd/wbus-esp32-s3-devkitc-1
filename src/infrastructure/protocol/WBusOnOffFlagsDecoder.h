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
        result.activeComponents = buildActiveComponentsString(result);

        return result;
    }

private:
    static String buildActiveComponentsString(const OnOffFlags &flags)
    {
        String components = "";
        if (flags.combustionAirFan)
            components += "Вентилятор горения, ";
        if (flags.glowPlug)
            components += "Свеча накаливания, ";
        if (flags.fuelPump)
            components += "Топливный насос, ";
        if (flags.circulationPump)
            components += "Циркуляционный насос, ";
        if (flags.vehicleFanRelay)
            components += "Вентилятор автомобиля, ";
        if (flags.nozzleStockHeating)
            components += "Подогрев форсунки, ";
        if (flags.flameIndicator)
            components += "Индикатор пламени, ";

        if (components.length() > 0)
        {
            return components.substring(0, components.length() - 2);
        }
        return "нет активных";
    }
};