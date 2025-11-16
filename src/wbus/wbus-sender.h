
#ifndef WBUS_SENDER_H
#define WBUS_SENDER_H

#include <Arduino.h>
#include "common/constants.h"

struct WBusPacket
{
  byte data[MESSAGE_BUFFER_SIZE];
  unsigned long byteCount;
};

bool validateWbusPacket(WBusPacket packet);
WBusPacket parseHexStringToPacket(String input);
bool sendWbusCommand(String command);

#endif // WBUS_SENDER_H