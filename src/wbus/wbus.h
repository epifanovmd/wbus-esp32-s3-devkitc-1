// wbus.h

#ifndef WBUS_H
#define WBUS_H

#include <Arduino.h>
#include "common/constants.h"
#include "common/print/print.h"
#include "common/utils/utils.h"

enum WebastoState
{
    WBUS_STATE_OFF,
    WBUS_STATE_INITIALIZING,
    WBUS_STATE_READY,
    WBUS_STATE_HEATING,
    WBUS_STATE_VENTILATING,
    WBUS_STATE_ERROR
};

class WBus
{
private:
    WebastoState currentState = WBUS_STATE_OFF;

public:
    void init();
    void wakeUp();
    void connect();
    void processQueue();
    void processReceiver();

    WebastoState getState() { return currentState; }
};

extern WBus wBus;

#endif // WBUS_H