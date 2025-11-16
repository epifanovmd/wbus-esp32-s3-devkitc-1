#ifndef WBUS_SENSORS_H
#define WBUS_SENSORS_H

#include <Arduino.h>
#include "wbus-sensors-decoder.h"

class WebastoSensors
{
private:
    OperationalMeasurements operationalMeasurements;
    FuelSettings fuelSettings;
    OnOffFlags onOffFlags;
    StatusFlags statusFlags;
    OperatingState operatingState;
    SubsystemsStatus subsystemsStatus;

    // Обработчики ответов
    void handleOperationalInfoResponse(bool success, String cmd, String response);
    void handleFuelSettingsResponse(bool success, String cmd, String response);
    void handleOnOffFlagsResponse(bool success, String cmd, String response);
    void handleStatusFlagsResponse(bool success, String cmd, String response);
    void handleOperatingStateResponse(bool success, String cmd, String response);
    void handleSubsystemsStatusResponse(bool success, String cmd, String response);

public:
    // Запросы данных
    void getOperationalInfo(bool loop = false);
    void getFuelSettings(bool loop = false);
    void getOnOffFlags(bool loop = false);
    void getStatusFlags(bool loop = false);
    void getOperatingState(bool loop = false);
    void getSubsystemsStatus(bool loop = false);
    void getAllSensorData(bool loop = false);

    void startContinuousMonitoring();
    void stopContinuousMonitoring();
    void setLoopInterval(unsigned long interval);

    // Вывод данных
    void printSensorData();

    // Геттеры
    OperationalMeasurements getCurrentMeasurements() { return operationalMeasurements; }
    FuelSettings getFuelSettingsData() { return fuelSettings; }
    OnOffFlags getOnOffFlagsData() { return onOffFlags; }
    StatusFlags getStatusFlagsData() { return statusFlags; }
    OperatingState getOperatingStateData() { return operatingState; }
    SubsystemsStatus getSubsystemsStatusData() { return subsystemsStatus; }
};

extern WebastoSensors webastoSensors;

#endif // WBUS_SENSORS_H