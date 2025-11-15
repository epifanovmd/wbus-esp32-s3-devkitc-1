
// wbus.h

#ifndef WBUS_H
#define WBUS_H

#include <Arduino.h>
#include "common/constants.h"
#include "common/print/print.h"
#include "common/utils/utils.h"
#include "common/serial/serial.h"
#include "common/queue-map/queue-map.h"
#include "wbus/wbus-queue.h"
#include "wbus/wbus-received-data.h"
#include "wbus/wbus.constants.h"
#include "wbus/wbus-sender.h"
#include "wbus/wbus-decoders.h"
#include "wbus/wbus-callbacks.h"
#include "wbus/wbus-error.h"
#include "wbus/wbus-info.h"


void readWBusData();

void wakeUpWebasto();
void connectToWebasto();
void readErrors();

#endif // WBUS_H