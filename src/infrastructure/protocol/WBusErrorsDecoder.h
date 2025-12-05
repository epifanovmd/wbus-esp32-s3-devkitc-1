// src/infrastructure/protocol/WBusErrorsDecoder.h
#pragma once
#include <Arduino.h>
#include <map>
#include "../../domain/Entities.h"
#include "../../common/Utils.h"
#include "./WBusErrorType.h"
#include "../../common/PacketParser.h"

class WBusErrorsDecoder
{
private:
    WBusErrorMap &errorMap = WBusErrorMap::getInstance();

public:
    WBusErrorsDecoder() {}

    ErrorCollection decodeErrorPacket(const String &response)
    {
        ErrorCollection result;

        PacketParser parser;

        if (parser.parseFromString(response, WBusCommandBuilder::CMD_READ_ERRORS, PacketParser::WithIndex(WBusCommandBuilder::ERROR_READ_LIST), PacketParser::WithMinLength(4)))
        {
            auto &data = parser.getBytes();

            return decodeErrorList(&data[4], parser.getByteCounts() - 4);
        }

        return result;
    }

    ErrorCollection decodeErrorList(const uint8_t *data, uint8_t dataLength)
    {
        ErrorCollection result;

        if (dataLength < 1)
            return result;

        uint8_t errorCount = data[0];

        if (errorCount == 0)
        {
            return result;
        }

        if (dataLength < 1 + errorCount * 2)
        {
            return result;
        }

        for (int i = 0; i < errorCount; i++)
        {
            uint8_t errorCode = data[1 + i * 2];
            uint8_t counter = data[2 + i * 2];
            WebastoError error = decodeSingleError(errorCode, counter);
            result.addError(error);
        }

        return result;
    }

    WebastoError decodeSingleError(uint8_t errorCode, uint8_t counter)
    {
        WebastoError error(errorCode, counter);
        WBusErrorType err = errorMap.getError(errorCode);
        error.errorName = err.name;
        error.errorDescription = err.description;

        return error;
    }
};