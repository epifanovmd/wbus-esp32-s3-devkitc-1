// src/interfaces/ISensorManager.h
#pragma once
#include "../domain/Entities.h"

class ISensorManager
{
public:
    virtual ~ISensorManager() = default;

    virtual void requestAllSensorData(bool loop = false) = 0;
    virtual void requestOperationalInfo(bool loop = false, std::function<void(String tx, String rx, OperationalMeasurements* measurements)> callback = nullptr) = 0;
    virtual void requestFuelSettings(bool loop = false, std::function<void(String tx, String rx, FuelSettings* fuel)> callback = nullptr) = 0;
    virtual void requestOnOffFlags(bool loop = false, std::function<void(String tx, String rx, OnOffFlags* onOff)> callback = nullptr) = 0;
    virtual void requestStatusFlags(bool loop = false, std::function<void(String tx, String rx, StatusFlags* status)> callback = nullptr) = 0;
    virtual void requestOperatingState(bool loop = false, std::function<void(String tx, String rx, OperatingState* state)> callback = nullptr) = 0;
    virtual void requestSubsystemsStatus(bool loop = false, std::function<void(String tx, String rx, SubsystemsStatus* subsystems)> callback = nullptr) = 0;


    virtual OperationalMeasurements getOperationalMeasurementsData() = 0;
    virtual FuelSettings getFuelSettingsData() = 0;
    virtual OnOffFlags getOnOffFlagsData() = 0;
    virtual StatusFlags getStatusFlagsData() = 0;
    virtual OperatingState geToperatingStateData() = 0;
    virtual SubsystemsStatus geTsubsystemsStatusData() = 0;
};