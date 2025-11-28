#pragma once
#include <Arduino.h>
#include "../../common/Constants.h"
#include "../../common/Utils.h"

struct WBusCommand
{
    uint8_t data[MESSAGE_BUFFER_SIZE];
    size_t byteCount;

    WBusCommand(String &command)
    {
        command.trim();
        command.toUpperCase();

        int tempByteCount;
        uint8_t *byteArray = Utils::hexStringToByteArray(command, tempByteCount);

        byteCount = min(tempByteCount, MESSAGE_BUFFER_SIZE);
        memcpy(data, byteArray, byteCount);
    }

    WBusCommand(const uint8_t *inputData, size_t length)
    {
        byteCount = min((size_t)length, (size_t)MESSAGE_BUFFER_SIZE);
        memcpy(data, inputData, byteCount);
    }

    WBusCommand() : byteCount(0)
    {
        memset(data, 0, MESSAGE_BUFFER_SIZE);
    }

    String toString() const
    {
        return Utils::bytesToHexString(data, byteCount);
    }

    bool isValid() const
    {
        return validateHeader() && validateLength() && validateChecksum();
    }

    bool validateChecksum() const
    {
        return Utils::validateChecksum(data, byteCount);
    }

    bool validateLength() const
    {
        return byteCount > 2 && (byteCount - 2) == data[1];
    }

    bool validateHeader() const
    {
        uint8_t header = data[0];

        return header == TXHEADER && header == RXHEADER;
    }
};