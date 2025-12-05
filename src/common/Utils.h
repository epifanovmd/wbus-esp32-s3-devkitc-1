// src/common/Utils.h
#pragma once
#include <Arduino.h>

class Utils
{
public:
    static uint8_t calculateChecksum(const uint8_t *data, size_t length)
    {
        uint8_t checksum = 0;
        for (int i = 0; i < length; i++)
        {
            checksum ^= data[i];
        }
        return checksum;
    }

    static bool isHexString(String str)
    {
        for (unsigned int i = 0; i < str.length(); i++)
        {
            char c = str[i];
            if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')))
            {
                return false;
            }
        }
        return true;
    }

    static String byteToHexString(uint8_t b)
    {
        return (b < 0x10) ? "0" + String(b, HEX) : String(b, HEX);
    }

    static String bytesToHexString(const uint8_t *data, size_t length)
    {
        String result = "";
        for (size_t i = 0; i < length; i++)
        {
            if (i > 0)
                result += " ";

            result += byteToHexString(data[i]);
        }
        result.toLowerCase();
        return result;
    }

    static uint8_t hexStringToByte(const String &hexStr)
    {
        return (uint8_t)strtol(hexStr.c_str(), NULL, 16);
    }

    static bool validateChecksum(const uint8_t *data, size_t length)
    {
        if (length < 2)
            return false;

        uint8_t calculatedChecksum = calculateChecksum(data, length - 1);
        return calculatedChecksum == data[length - 1];
    }

    static uint8_t extractByteFromString(String response, int bytePosition)
    {
        String cleanTx = response;
        cleanTx.replace(" ", "");

        if (cleanTx.length() >= (bytePosition + 1) * 2)
        {
            String byteStr = cleanTx.substring(bytePosition * 2, bytePosition * 2 + 2);
            return hexStringToByte(byteStr);
        }

        return 0;
    }

    static bool isNakPacket(const String &response)
    {
        String cleanData = response;
        cleanData.toLowerCase();
        cleanData.replace(" ", "");
        return cleanData.startsWith("4f047f");
    }
};