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

    static bool hexStringToByteArray(const String &hexString, uint8_t *buffer, size_t bufferSize, int &byteCount)
    {
        byteCount = 0;
        String cleanString = hexString;
        cleanString.replace(" ", "");

        size_t maxBytes = (cleanString.length() + 1) / 2; // Округление вверх
        if (maxBytes > bufferSize)
        {
            return false;
        }

        for (size_t i = 0; i < cleanString.length(); i += 2)
        {
            if (byteCount >= bufferSize)
            {
                return false;
            }

            if (i + 2 <= cleanString.length())
            {
                buffer[byteCount++] = hexStringToByte(cleanString.substring(i, i + 2));
            }
            else
            {
                buffer[byteCount++] = hexStringToByte(cleanString.substring(i, i + 1) + "0");
            }
        }

        return true;
    }

    static bool validateChecksum(const String &hexData)
    {
        String cleanData = hexData;
        cleanData.replace(" ", "");

        if (!isHexString(cleanData) || cleanData.length() < 4)
        {
            return false;
        }

        uint8_t data[MESSAGE_BUFFER_SIZE];
        int byteCount;
        Utils::hexStringToByteArray(cleanData, data, sizeof(data), byteCount);

        if (byteCount < 2)
            return false;

        uint8_t calculatedChecksum = calculateChecksum(data, byteCount - 1);
        return calculatedChecksum == data[byteCount - 1];
    }

    static bool validateChecksum(const uint8_t *data, size_t length)
    {
        if (length < 2)
            return false;

        uint8_t calculatedChecksum = calculateChecksum(data, length - 1);
        return calculatedChecksum == data[length - 1];
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