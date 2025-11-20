#ifndef WBUS_SENSORS_H
#define WBUS_SENSORS_H

#include <Arduino.h>
#include <ArduinoJson.h>
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

public:
    OperationalMeasurements *handleOperationalInfoResponse(String rx);
    FuelSettings *handleFuelSettingsResponse(String rx);
    OnOffFlags *handleOnOffFlagsResponse(String rx);
    StatusFlags *handleStatusFlagsResponse(String rx);
    OperatingState *handleOperatingStateResponse(String rx);
    SubsystemsStatus *handleSubsystemsStatusResponse(String rx);

    void getOperationalInfo(bool loop = false, std::function<void(String, String, OperationalMeasurements*)> callback = nullptr);
    void getFuelSettings(bool loop = false, std::function<void(String, String, FuelSettings*)> callback = nullptr);
    void getOnOffFlags(bool loop = false, std::function<void(String, String, OnOffFlags*)> callback = nullptr);
    void getStatusFlags(bool loop = false, std::function<void(String, String, StatusFlags*)> callback = nullptr);
    void getOperatingState(bool loop = false, std::function<void(String, String, OperatingState*)> callback = nullptr);
    void getSubsystemsStatus(bool loop = false, std::function<void(String, String, SubsystemsStatus*)> callback = nullptr);

    // Функции формирования JSON
    String createJsonOperationalInfo(const OperationalMeasurements& data);
    String createJsonFuelSettings(const FuelSettings& data);
    String createJsonOnOffFlags(const OnOffFlags& data);
    String createJsonStatusFlags(const StatusFlags& data);
    String createJsonOperatingState(const OperatingState& data);
    String createJsonSubsystemsStatus(const SubsystemsStatus& data);

    // Перегруженные версии без параметров (используют внутренние данные)
    String createJsonOperationalInfo();
    String createJsonFuelSettings();
    String createJsonOnOffFlags();
    String createJsonStatusFlags();
    String createJsonOperatingState();
    String createJsonSubsystemsStatus();

    void stopMonitoring();
    void clear();

    // Вывод данных
    void printSensorData();

    // Геттеры
    OperationalMeasurements* getOperationalInfoData() { return &operationalMeasurements; }
    FuelSettings* getFuelSettingsData() { return &fuelSettings; }
    OnOffFlags* getOnOffFlagsData() { return &onOffFlags; }
    StatusFlags* getStatusFlagsData() { return &statusFlags; }
    OperatingState* getOperatingStateData() { return &operatingState; }
    SubsystemsStatus* getSubsystemsStatusData() { return &subsystemsStatus; }
};

extern WebastoSensors webastoSensors;

#endif // WBUS_SENSORS_H