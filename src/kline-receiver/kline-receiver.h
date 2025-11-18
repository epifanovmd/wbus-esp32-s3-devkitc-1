#ifndef KLINERECEIVER_H
#define KLINERECEIVER_H

#include "common/serial/serial.h"
#include "common/constants.h"
#include "kline-received-data.h"

class KLineReceiver
{
public:
    KLineReceivedData kLineReceivedData;

    void process();
};

extern KLineReceiver kLineReceiver;

#endif // KLINERECEIVER_H