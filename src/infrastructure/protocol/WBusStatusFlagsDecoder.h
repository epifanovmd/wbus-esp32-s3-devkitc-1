// src/infrastructure/protocol/WBusStatusFlagsDecoder.h
#pragma once
#include <Arduino.h>
#include "../../domain/Entities.h"
#include "../../common/Utils.h"

class WBusStatusFlagsDecoder
{
public:
    static StatusFlags decode(const String &response)
    {
        StatusFlags result;

        if (!Utils::validateASCPacketStructure(response, 0x50, 0x02, 10))
        {
            return result;
        }

        uint8_t data[MESSAGE_BUFFER_SIZE];
        int byteCount;
        Utils::hexStringToByteArray(response, data, sizeof(data), byteCount);

        if (byteCount >= 10)
        {
            // Байт 0
            result.mainSwitch = (data[4] & 0x01) != 0;
            result.supplementalHeatRequest = (data[4] & 0x10) != 0;
            result.parkingHeatRequest = (data[4] & 0x20) != 0;
            result.ventilationRequest = (data[4] & 0x40) != 0;

            // Байт 1
            result.summerMode = (data[5] & 0x01) != 0;
            result.externalControl = (data[5] & 0x02) != 0;

            // Байт 2
            result.generatorSignal = (data[6] & 0x10) != 0;

            // Байт 3
            result.boostMode = (data[7] & 0x10) != 0;
            result.auxiliaryDrive = (data[7] & 0x01) != 0;

            // Байт 4
            result.ignitionSignal = (data[8] & 0x01) != 0;
        }

        return result;
    }

private:
};