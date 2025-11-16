#ifndef WBUS_ERROR_H
#define WBUS_ERROR_H

#include <Arduino.h>
#include <functional>
#include "wbus-queue.h"
#include "wbus.constants.h"
#include "wbus-error-codes.h"

class WebastoError
{
private:
    WebastoErrorCodes errorCodes;

    void hexStringToBytes(const String &hexString, uint8_t *output, int maxLength);
    String decodeErrorPacket(const String &packet);
    String decodeErrorList(const uint8_t *data, uint8_t dataLength);
    String decodeSingleError(uint8_t errorCode, uint8_t counter = 0);

public:
    void check();
    void clear();
};

extern WebastoError webastoError;

#endif // WBUS_ERROR_H