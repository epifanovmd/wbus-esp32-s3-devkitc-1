// src/infrastructure/protocol/WBusOperationalInfoDecoder.h
#pragma once
#include <Arduino.h>
#include "../../domain/Entities.h"
#include "../../common/Utils.h"

class WBusOperationalInfoDecoder
{
public:
    static OperationalMeasurements decode(const String &response)
    {
        OperationalMeasurements result;

        if (!Utils::validateASCPacketStructure(response, 0x50, 0x05, 13))
        {
            return result;
        }

        uint8_t data[MESSAGE_BUFFER_SIZE];
        int byteCount;
        Utils::hexStringToByteArray(response, data, sizeof(data), byteCount);

        if (byteCount >= 13)
        {
            result.temperature = data[4] - 50.0;
            result.voltage = (float)((data[5] << 8) | data[6]) / 1000.0;
            result.flameDetected = (data[7] == 0x01);
            result.heatingPower = (data[8] << 8) | data[9];
            result.flameResistance = (data[10] << 8) | data[11];
        }

        return result;
    }
};