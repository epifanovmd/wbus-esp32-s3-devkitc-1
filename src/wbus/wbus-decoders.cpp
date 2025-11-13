#include "wbus/wbus-decoders.h"

String WBusDecoders::decodeRawResponse(const String &hexResponse)
{
  String result = "";
  String cleanHex = hexResponse;
  cleanHex.replace(" ", "");

  for (size_t i = 0; i < cleanHex.length(); i += 2)
  {
    String byteStr = cleanHex.substring(i, i + 2);
    byte value = hexStringToByte(byteStr);

    // Пропускаем служебные байты (header, length, checksum)
    if (i == 0 || i == 2 || i >= cleanHex.length() - 2)
    {
      continue;
    }

    // Печатаемые ASCII символы
    if (value >= 32 && value <= 126)
    {
      result += (char)value;
    }
    else
    {
      result += "[" + byteStr + "]";
    }
  }

  return result;
}

SensorStatusFlags WBusDecoders::decodeStatusFlags(const String &hexData)
{
  SensorStatusFlags flags = {};
  String cleanData = hexData;
  cleanData.replace(" ", "");

  if (cleanData.length() >= 8)
  {
    byte byte0 = hexStringToByte(cleanData.substring(0, 2));
    byte byte1 = hexStringToByte(cleanData.substring(2, 4));
    byte byte2 = hexStringToByte(cleanData.substring(4, 6));
    byte byte3 = hexStringToByte(cleanData.substring(6, 8));

    flags.supplementalHeaterRequest = getBit(byte0, 4); // 0x10
    flags.mainSwitch = getBit(byte0, 0);                // 0x01
    flags.summerSeason = getBit(byte1, 0);              // 0x01
    flags.generatorSignal = getBit(byte2, 4);           // 0x10
    flags.boostMode = getBit(byte3, 4);                 // 0x10
    flags.auxiliaryDrive = getBit(byte3, 0);            // 0x01

    if (cleanData.length() >= 10)
    {
      byte byte4 = hexStringToByte(cleanData.substring(8, 10));
      flags.ignition = getBit(byte4, 0); // 0x01
    }
  }

  return flags;
}

OnOffFlags WBusDecoders::decodeOnOffFlags(const String &hexData)
{
  OnOffFlags flags = {};
  String cleanData = hexData;
  cleanData.replace(" ", "");

  if (cleanData.length() >= 2)
  {
    byte data = hexStringToByte(cleanData.substring(0, 2));

    flags.combustionAirFan = getBit(data, 0);   // 0x01
    flags.glowPlug = getBit(data, 1);           // 0x02
    flags.fuelPump = getBit(data, 2);           // 0x04
    flags.circulationPump = getBit(data, 3);    // 0x08
    flags.vehicleFanRelay = getBit(data, 4);    // 0x10
    flags.nozzleStockHeating = getBit(data, 5); // 0x20
    flags.flameIndicator = getBit(data, 6);     // 0x40
  }

  return flags;
}

OperationalMeasurements WBusDecoders::decodeOperationalMeasurements(const String &hexData)
{
  OperationalMeasurements measurements = {};

  // Разбиваем строку на байты
  int byteCount = 0;
  byte data[20];
  String cleanData = hexData;
  cleanData.replace(" ", "");

  for (int i = 0; i < cleanData.length(); i += 2)
  {
    if (byteCount < 20)
    {
      data[byteCount++] = hexStringToByte(cleanData.substring(i, i + 2));
    }
  }

  // Проверяем что это ответ на команду 0x50 индекс 0x05
  if (byteCount >= 13 && data[2] == 0xD0 && data[3] == 0x05)
  {
    // Данные начинаются с 4-го байта
    measurements.temperature = decodeTemperature(data[4]);             // 09
    measurements.voltage = (float)((data[5] << 8) | data[6]) / 1000.0; // 2F 8A
    measurements.flameDetected = (data[7] == 0x01);                    // 00
    measurements.heatingPower = (data[8] << 8) | data[9];              // 00 00
    measurements.flameResistance = (data[10] << 8) | data[11];         // 03 E8
  }

  return measurements;
}

OperatingTimes WBusDecoders::decodeOperatingTimes(const String &hexData)
{
  OperatingTimes times = {};
  String cleanData = hexData;
  cleanData.replace(" ", "");

  if (cleanData.length() >= 16)
  {
    times.workingHours = (hexStringToByte(cleanData.substring(0, 2)) << 8) |
                         hexStringToByte(cleanData.substring(2, 4));
    times.workingMinutes = hexStringToByte(cleanData.substring(4, 6));
    times.operatingHours = (hexStringToByte(cleanData.substring(6, 8)) << 8) |
                           hexStringToByte(cleanData.substring(8, 10));
    times.operatingMinutes = hexStringToByte(cleanData.substring(10, 12));
    times.startCounter = (hexStringToByte(cleanData.substring(12, 14)) << 8) |
                         hexStringToByte(cleanData.substring(14, 16));
  }

  return times;
}

SubsystemsStatus WBusDecoders::decodeSubsystemsStatus(const String &hexData)
{
  SubsystemsStatus status = {};
  String cleanData = hexData;
  cleanData.replace(" ", "");

  if (cleanData.length() >= 10)
  {
    status.glowPlugPower = hexStringToByte(cleanData.substring(0, 2)) / 2;         // % * 2
    status.fuelPumpFrequency = hexStringToByte(cleanData.substring(2, 4)) / 2;     // Hz * 2
    status.combustionFanPower = hexStringToByte(cleanData.substring(4, 6)) / 2;    // % * 2
    status.circulationPumpPower = hexStringToByte(cleanData.substring(8, 10)) / 2; // % * 2
  }

  return status;
}

String WBusDecoders::decodeOperatingState(byte stateCode)
{
  switch (stateCode)
  {
  case 0x00:
    return "Burn out";
  case 0x01:
    return "Deactivation";
  case 0x04:
    return "Off state";
  case 0x05:
    return "Combustion process part load";
  case 0x06:
    return "Combustion process full load";
  case 0x16:
    return "Initialization";
  case 0x1c:
    return "Ventilation";
  case 0x41:
    return "Combustion process parking heating";
  case 0x42:
    return "Combustion process suppl. heating";
  default:
    return "Unknown state: 0x" + String(stateCode, HEX);
  }
}

ErrorInfo WBusDecoders::decodeErrorInfo(const String &hexData)
{
  ErrorInfo error = {};
  String cleanData = hexData;
  cleanData.replace(" ", "");

  if (cleanData.length() >= 28)
  {
    error.errorCode = hexStringToByte(cleanData.substring(0, 2));
    error.count = hexStringToByte(cleanData.substring(2, 4));

    byte status = hexStringToByte(cleanData.substring(4, 6));
    error.isStored = getBit(status, 0); // 0x01
    error.isActual = getBit(status, 1); // 0x02

    error.operatingState = hexStringToByte(cleanData.substring(8, 10));
    error.temperature = decodeTemperature(hexStringToByte(cleanData.substring(12, 14)));

    // Напряжение при ошибке
    error.voltage = decodeVoltage(
                        hexStringToByte(cleanData.substring(14, 16)),
                        hexStringToByte(cleanData.substring(16, 18))) /
                    1000.0;

    // Время работы при ошибке
    error.operatingHours = (hexStringToByte(cleanData.substring(18, 20)) << 8) |
                           hexStringToByte(cleanData.substring(20, 22));
    error.operatingMinutes = hexStringToByte(cleanData.substring(22, 24));
  }

  return error;
}

String WBusDecoders::decodeWbusCode(const String &hexData)
{
  String result = "WBUS Code: ";
  String cleanData = hexData;
  cleanData.replace(" ", "");

  if (cleanData.length() >= 14)
  {
    byte byte0 = hexStringToByte(cleanData.substring(0, 2));
    byte byte1 = hexStringToByte(cleanData.substring(2, 4));

    if (getBit(byte0, 3))
      result += "SimpleOnOff ";
    if (getBit(byte0, 4))
      result += "ParkHeat ";
    if (getBit(byte0, 5))
      result += "SuppHeat ";
    if (getBit(byte0, 6))
      result += "Ventilation ";
    if (getBit(byte0, 7))
      result += "Boost ";

    if (getBit(byte1, 1))
      result += "ExtCircPump ";
    if (getBit(byte1, 2))
      result += "CombustionFan ";
    if (getBit(byte1, 3))
      result += "GlowPlug ";
    if (getBit(byte1, 4))
      result += "FuelPump ";
    if (getBit(byte1, 5))
      result += "CircPump ";
    if (getBit(byte1, 6))
      result += "VehicleFan ";
    if (getBit(byte1, 7))
      result += "YellowLED ";
  }

  return result;
}

String WBusDecoders::decodeWbusVersion(byte versionByte)
{
  int major = (versionByte >> 4) & 0x0F;
  int minor = versionByte & 0x0F;
  return String(major) + "." + String(minor);
}

float WBusDecoders::decodeTemperature(byte tempByte)
{
  return (tempByte - 50.0); // Offset +50°C
}

float WBusDecoders::decodeVoltage(byte highByte, byte lowByte)
{
  return (float)((highByte << 8) | lowByte); // mV
}

// =============================================
// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ
// =============================================

byte WBusDecoders::hexStringToByte(const String &hexStr)
{
  return (byte)strtol(hexStr.c_str(), NULL, 16);
}

int WBusDecoders::hexStringToInt(const String &hexStr)
{
  return (int)strtol(hexStr.c_str(), NULL, 16);
}

String WBusDecoders::byteToBinaryString(byte b)
{
  String result = "";
  for (int i = 7; i >= 0; i--)
  {
    result += getBit(b, i) ? "1" : "0";
  }
  return result;
}

bool WBusDecoders::getBit(byte data, byte bitPosition)
{
  return (data >> bitPosition) & 0x01;
}