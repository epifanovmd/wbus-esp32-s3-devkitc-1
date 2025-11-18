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
    void handleOperationalInfoResponse(String tx, String rx);
    void handleFuelSettingsResponse(String tx, String rx);
    void handleOnOffFlagsResponse(String tx, String rx);
    void handleStatusFlagsResponse(String tx, String rx);
    void handleOperatingStateResponse(String tx, String rx);
    void handleSubsystemsStatusResponse(String tx, String rx);

public:
    // Запросы данных
    void getOperationalInfo(bool loop = false, std::function<void(String, String)> callback = nullptr);
    void getFuelSettings(bool loop = false, std::function<void(String, String)> callback = nullptr);
    void getOnOffFlags(bool loop = false, std::function<void(String, String)> callback = nullptr);
    void getStatusFlags(bool loop = false, std::function<void(String, String)> callback = nullptr);
    void getOperatingState(bool loop = false, std::function<void(String, String)> callback = nullptr);
    void getSubsystemsStatus(bool loop = false, std::function<void(String, String)> callback = nullptr);

    // универсальная функция обработки, по tx выбирает нужный обработчик
    bool handleCommandResponse(String tx, String rx);

    void stopMonitoring();
    void clear();

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