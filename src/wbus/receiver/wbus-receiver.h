#include "common/serial/serial.h"
#include "common/constants.h"
#include "wbus-received-data.h"

class WBusReceiver
{
public:
    WBusReceivedData wBusReceivedData;

    void process();
};

extern WBusReceiver wBusReceiver;