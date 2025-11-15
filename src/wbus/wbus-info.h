#ifndef WBUS_INFO_H
#define WBUS_INFO_H

#include <Arduino.h>
#include "wbus/wbus-queue.h"
#include "wbus/wbus.constants.h"

class WebastoInfo
{
public:
    static void getWBusVersion();
    static void getDeviceName();
    static void getWBusCode();
    static void getDeviceID();
    static void getHeaterManufactureDate();
    static void getControllerManufactureDate();
    static void getCustomerID();
    static void getSerialNumber();

private:
};

extern WebastoInfo webastoInfo;

#endif // WBUS_INFO_H