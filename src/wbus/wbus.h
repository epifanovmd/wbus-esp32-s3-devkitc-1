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
#include "wbus/wbus-sensors.h"
#include "wbus/wbus-error.h"
#include "wbus/wbus-info.h"


void readWBusData();

void wakeUpWebasto();
void connectToWebasto();
void readErrors();

enum WebastoState {
    WBUS_STATE_OFF,
    WBUS_STATE_INITIALIZING,
    WBUS_STATE_READY,
    WBUS_STATE_HEATING,
    WBUS_STATE_VENTILATING,
    WBUS_STATE_ERROR
};

class WebastoStateMachine {
private:
    WebastoState currentState = WBUS_STATE_OFF;
    
public:
    void updateState(String sensorData) {
        // Логика определения состояния на основе данных датчиков
    }
    
    WebastoState getState() { return currentState; }
};

#endif // WBUS_H