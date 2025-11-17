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