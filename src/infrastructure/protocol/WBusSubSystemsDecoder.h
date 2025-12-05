// src/infrastructure/protocol/WBusSubSystemsDecoder.h
#pragma once
#include <Arduino.h>
#include "../../domain/Entities.h"
#include "../../common/Utils.h"

class WBusSubSystemsDecoder
{
public:
    static SubsystemsStatus decode(const String &response)
    {
        SubsystemsStatus result;

        PacketParser parser;

        if (parser.parseFromString(response, WBusCommandBuilder::CMD_READ_SENSOR, PacketParser::WithIndex(WBusCommandBuilder::SENSOR_SUBSYSTEMS_STATUS), PacketParser::WithMinLength(10)))
        {
            auto &data = parser.getBytes();

            result.glowPlugPower = data[4];
            result.fuelPumpFrequency = data[5];
            result.combustionFanPower = data[6];
            result.unknownByte3 = data[7];
            result.circulationPumpPower = data[8];

            result.glowPlugPowerPercent = result.glowPlugPower / 2.0;
            result.fuelPumpFrequencyHz = result.fuelPumpFrequency / 2.0;
            result.combustionFanPowerPercent = result.combustionFanPower / 2.0;
            result.circulationPumpPowerPercent = result.circulationPumpPower / 2.0;
        }

        Serial.println();
        Serial.println("JSON : " + result.toJson());

        return result;
    }
};