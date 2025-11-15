// utils.h

#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include "common/constants.h"

byte calculateChecksum(byte *data, int length);
bool isHexString(String str);
String byteToHexString(byte b);
byte hexStringToByte(const String &hexStr);

#endif // UTILS_H