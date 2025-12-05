// src/infrastructure/protocol/WBusFuelSettingsDecoder.h
#pragma once
#include <Arduino.h>
#include "../../domain/Entities.h"
#include "../../common/Utils.h"
#include "../../common/PacketParser.h"

class WBusFuelSettingsDecoder
{
public:
  static FuelSettings decode(const String &response)
  {
    FuelSettings result;

    PacketParser parser;

    if (parser.parseFromString(response, WBusCommandBuilder::CMD_READ_SENSOR, PacketParser::WithIndex(WBusCommandBuilder::SENSOR_FUEL_SETTINGS), PacketParser::WithMinLength(8)))
    {
      auto &data = parser.getBytes();

      result.fuelType = data[4];
      result.maxHeatingTime = data[5];
      result.ventilationFactor = data[6];
      result.fuelTypeName = determineFuelTypeName(result.fuelType);
    }

    return result;
  }

private:
  static String determineFuelTypeName(uint8_t fuelType)
  {
    switch (fuelType)
    {
    case 0x0D:
      return "Дизельное топливо";
    case 0x1D:
      return "Дизельное топливо (альтернативный код)";
    case 0x2D:
      return "Бензин";
    case 0x03:
      return "Газ";
    case 0x05:
      return "Биотопливо";
    default:
      if (fuelType >= 0x01 && fuelType <= 0x0F)
        return "Дизельные топлива";
      if (fuelType >= 0x10 && fuelType <= 0x2F)
        return "Бензины";
      if (fuelType >= 0x30 && fuelType <= 0x4F)
        return "Газовые топлива";
      return "Неизвестный тип";
    }
  }
};