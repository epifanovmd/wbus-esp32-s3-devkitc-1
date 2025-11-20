#include "wbus-sniffer.h"
#include "wbus/wbus-queue.h"
#include "kline-receiver/kline-receiver.h"
#include "server/socket-server.h"
#include "wbus/wbus.constants.h"
#include "wbus/wbus-info.h"
#include "wbus/wbus-sensors.h"

WBusSniffer wBusSniffer;

void WBusSniffer::handleInfoCommands(String tx, String rx)
{
    if (tx == CMD_READ_INFO_WBUS_VERSION)
    {
        webastoInfo.handleWBusVersionResponse(rx);
    }
    else if (tx == CMD_READ_INFO_DEVICE_NAME)
    {
        webastoInfo.handleDeviceNameResponse(rx);
    }
    else if (tx == CMD_READ_INFO_WBUS_CODE)
    {
        webastoInfo.handleWBusCodeResponse(rx);
    }
    else if (tx == CMD_READ_INFO_DEVICE_ID)
    {
        webastoInfo.handleDeviceIDResponse(rx);
    }
    else if (tx == CMD_READ_INFO_HEATER_MFG_DATE)
    {
        webastoInfo.handleHeaterManufactureDateResponse(rx);
    }
    else if (tx == CMD_READ_INFO_CTRL_MFG_DATE)
    {
        webastoInfo.handleControllerManufactureDateResponse(rx);
    }
    else if (tx == CMD_READ_INFO_CUSTOMER_ID)
    {
        webastoInfo.handleCustomerIDResponse(rx);
    }
    else if (tx == CMD_READ_INFO_SERIAL_NUMBER)
    {
        webastoInfo.handleSerialNumberResponse(rx);
    }
}

void WBusSniffer::handleSensorsCommands(String tx, String rx)
{
    if (tx == CMD_READ_SENSOR_OPERATIONAL)
    {
        webastoSensors.handleOperationalInfoResponse(rx);
    }
    else if (tx == CMD_READ_SENSOR_FUEL_SETTINGS)
    {
        webastoSensors.handleFuelSettingsResponse(rx);
    }
    else if (tx == CMD_READ_SENSOR_ON_OFF_FLAGS)
    {
        webastoSensors.handleOnOffFlagsResponse(rx);
    }
    else if (tx == CMD_READ_SENSOR_STATUS_FLAGS)
    {
        webastoSensors.handleStatusFlagsResponse(rx);
    }
    else if (tx == CMD_READ_SENSOR_OPERATING_STATE)
    {
        webastoSensors.handleOperatingStateResponse(rx);
    }
    else if (tx == CMD_READ_SENSOR_SUBSYSTEMS_STATUS)
    {
        webastoSensors.handleSubsystemsStatusResponse(rx);
    }
}

void WBusSniffer::process()
{
    if (kLineReceiver.kLineReceivedData.isTxReceived())
    {
        currentTx = kLineReceiver.kLineReceivedData.txString;
    }

    if (kLineReceiver.kLineReceivedData.isRxReceived())
    {
        handleInfoCommands(currentTx, kLineReceiver.kLineReceivedData.rxString);
        handleSensorsCommands(currentTx, kLineReceiver.kLineReceivedData.rxString);
        currentTx = String();
    }
}