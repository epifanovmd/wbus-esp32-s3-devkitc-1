// src/common/Utils.h
#pragma once
#include <Arduino.h>

class Utils
{
public:
    static byte calculateChecksum(byte *data, int length)
    {
        byte checksum = 0;
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

    static String byteToHexString(byte b)
    {
        return (b < 0x10) ? "0" + String(b, HEX) : String(b, HEX);
    }

    static byte hexStringToByte(const String &hexStr)
    {
        return (byte)strtol(hexStr.c_str(), NULL, 16);
    }

    static byte *hexStringToByteArray(const String &hexString, int &byteCount)
    {
        static byte data[32];
        byteCount = 0;

        String cleanString = hexString;
        cleanString.replace(" ", "");

        for (int i = 0; i < cleanString.length(); i += 2)
        {
            if (byteCount < 32 && i + 2 <= cleanString.length())
            {
                data[byteCount++] = hexStringToByte(cleanString.substring(i, i + 2));
            }
        }

        return data;
    }

    static void printHex(byte value, bool newLine)
    {
        if (value < 0x10)
            Serial.print("0");
        Serial.print(value, HEX);
        if (newLine)
            Serial.println();
        else
            Serial.print(" ");
    }
    static bool validateASCPacketStructure(const String &response, uint8_t expectedCommand, uint8_t expectedIndex, int minLength)
    {
        String cleanData = response;
        cleanData.replace(" ", "");

        if (cleanData.length() < minLength * 2)
        {
            return false;
        }

        if (cleanData.substring(0, 2) != "4f")
        {
            return false;
        }

        // Проверяем команду (с установленным битом ACK)
        uint8_t receivedCommand = Utils::hexStringToByte(cleanData.substring(4, 6));
        if (receivedCommand != (expectedCommand | 0x80))
        {
            return false;
        }

        // Проверяем индекс
        uint8_t receivedIndex = Utils::hexStringToByte(cleanData.substring(6, 8));
        if (receivedIndex != expectedIndex)
        {
            return false;
        }

        return true;
    }
};