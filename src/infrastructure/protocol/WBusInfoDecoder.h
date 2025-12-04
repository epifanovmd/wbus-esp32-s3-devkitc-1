// src/infrastructure/protocol/WBusInfoDecoder.h
#pragma once
#include <Arduino.h>
#include "../../domain/Entities.h"
#include "../../common/Utils.h"

class WBusInfoDecoder
{
public:
    static String decodeWBusVersion(const String &response)
    {
        String result = "";

        String versionData = extractDataFromResponse(response, "d10a", 2);
        if (versionData.length() == 2)
        {
            uint8_t versionByte = Utils::hexStringToByte(versionData);

            result = String((versionByte >> 4) & 0x0F) + "." + String(versionByte & 0x0F);
        }

        return result;
    }

    static String decodeDeviceName(const String &response)
    {
        String result = "";

        String nameData = extractDataFromResponse(response, "d10b");
        if (nameData.length() > 0)
        {
            String text = hexToString(nameData);
            text.trim();
            result = text;
        }

        return result;
    }

    static DecodedWBusCode decodeWBusCode(const String &response)
    {
        DecodedWBusCode result;
        WBusCodeFlags flags = {};

        String codeData = extractDataFromResponse(response, "d10c");
        if (codeData.length() >= 14)
        { // Need at least 7 bytes
            result.codeString = codeData;

            // Parse each byte
            for (int byteNum = 0; byteNum < 7 && (byteNum * 2 + 2) <= codeData.length(); byteNum++)
            {
                String byteStr = codeData.substring(byteNum * 2, byteNum * 2 + 2);
                uint8_t byteVal = Utils::hexStringToByte(byteStr);
                parseWBusCodeByte(byteNum, byteVal, flags);
            }

            result.flags = flags;
        }

        return result;
    }

    static String decodeDeviceID(const String &response)
    {
        String result = "";

        String idData = extractDataFromResponse(response, "d101");
        if (idData.length() > 0)
        {
            result = idData;
        }

        return result;
    }

    static DecodedManufactureDate decodeHeaterManufactureDate(const String &response)
    {
        return decodeManufactureDate(response, "d105");
    }

    static DecodedManufactureDate decodeControllerManufactureDate(const String &response)
    {
        return decodeManufactureDate(response, "d104");
    }

    static String decodeCustomerID(const String &response)
    {
        String result = "";

        String data = extractDataFromResponse(response, "d107");
        if (data.length() > 0)
        {
            result = hexToString(data);
        }

        return result;
    }

    static String decodeSerialNumber(const String &response)
    {
        String result = "";

        String data = extractDataFromResponse(response, "d109");
        if (data.length() >= 10)
        {
            result = data.substring(0, 10);
        }

        return result;
    }

private:
    static String extractDataFromResponse(const String &response, const String &command, int dataLength = 0)
    {
        String cleanData = response;
        cleanData.replace(" ", "");
        cleanData.toLowerCase();

        int start = cleanData.indexOf(command);
        if (start == -1)
        {
            return "";
        }

        start += command.length();

        if (dataLength > 0)
        {
            if (cleanData.length() >= start + dataLength)
            {
                return cleanData.substring(start, start + dataLength);
            }
        }
        else
        {
            if (cleanData.length() >= start + 2)
            {
                return cleanData.substring(start, cleanData.length() - 2);
            }
        }

        return "";
    }

    static String hexToString(const String &hexData)
    {
        String result = "";
        for (int i = 0; i < hexData.length(); i += 2)
        {
            if (i + 2 <= hexData.length())
            {
                String byteStr = hexData.substring(i, i + 2);
                if (byteStr == "00")
                    break;
                char c = (char)strtoul(byteStr.c_str(), NULL, 16);
                if (c >= 32 && c <= 126)
                {
                    result += c;
                }
            }
        }
        return result;
    }

    static DecodedManufactureDate decodeManufactureDate(const String &response, const String &command)
    {
        DecodedManufactureDate result = {"N/A", 0, 0, 0};

        String dateData = extractDataFromResponse(response, command, 6);
        if (dateData.length() == 6)
        {
            String dayStr = dateData.substring(0, 2);
            String monthStr = dateData.substring(2, 4);
            String yearStr = dateData.substring(4, 6);

            result.day = Utils::hexStringToByte(dayStr);
            result.month = Utils::hexStringToByte(monthStr);
            result.year = 2000 + Utils::hexStringToByte(yearStr);
            result.dateString = dayStr + "." + monthStr + ".20" + yearStr;
        }

        return result;
    }

    static void parseWBusCodeByte(uint8_t byteNum, uint8_t byteVal, WBusCodeFlags &flags)
    {
        switch (byteNum)
        {
        case 0: // Byte 0
            // flags.ZH = (byteVal & 0x01) != 0;
            flags.simpleOnOffControl = (byteVal & 0x08) != 0;
            flags.parkingHeating = (byteVal & 0x10) != 0;
            flags.supplementalHeating = (byteVal & 0x20) != 0;
            flags.ventilation = (byteVal & 0x40) != 0;
            flags.boostMode = (byteVal & 0x80) != 0;
            break;

        case 1: // Byte 1
            flags.externalCirculationPumpControl = (byteVal & 0x02) != 0;
            flags.combustionAirFan = (byteVal & 0x04) != 0;
            flags.glowPlug = (byteVal & 0x08) != 0;
            flags.fuelPump = (byteVal & 0x10) != 0;
            flags.circulationPump = (byteVal & 0x20) != 0;
            flags.vehicleFanRelay = (byteVal & 0x40) != 0;
            flags.yellowLED = (byteVal & 0x80) != 0;
            break;

        case 2: // Byte 2
            flags.greenLED = (byteVal & 0x01) != 0;
            flags.sparkTransmitter = (byteVal & 0x02) != 0;
            flags.solenoidValve = (byteVal & 0x04) != 0;
            flags.auxiliaryDriveIndicator = (byteVal & 0x08) != 0;
            flags.generatorSignalDPlus = (byteVal & 0x10) != 0;
            flags.fanInRPM = (byteVal & 0x20) != 0;
            // flags.ZH2 = (byteVal & 0x40) != 0;
            // flags.ZH3 = (byteVal & 0x80) != 0;
            break;

        case 3: // Byte 3
            flags.CO2Calibration = (byteVal & 0x02) != 0;
            flags.operationIndicator = (byteVal & 0x08) != 0;
            break;

        case 4: // Byte 4
            // flags.ZH4 = (byteVal & 0x0F);
            flags.powerInWatts = (byteVal & 0x10) != 0;
            // flags.ZH5 = (byteVal & 0x20) != 0;
            flags.flameIndicator = (byteVal & 0x40) != 0;
            flags.nozzleStockHeating = (byteVal & 0x80) != 0;
            break;

        case 5: // Byte 5
            flags.temperatureThresholds = (byteVal & 0x40) != 0;
            flags.ignitionFlag = (byteVal & 0x20) != 0;
            flags.fuelPrewarmingReadable = (byteVal & 0x80) != 0;
            break;

        case 6: // Byte 6
            flags.setValuesAvailable = (byteVal & 0x02) != 0;
            break;
        }
    }
};