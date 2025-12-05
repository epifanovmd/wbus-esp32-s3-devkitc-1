// src/interfaces/ISensorManager.h
#pragma once

class ISensorManager
{
public:
    virtual ~ISensorManager() = default;

    virtual void requestAllSensorData(bool loop = false) = 0;

    virtual void requestStatusFlags(bool loop = false) = 0;
    virtual void requestOnOffFlags(bool loop = false) = 0;
    virtual void requestFuelSettings(bool loop = false) = 0;
    virtual void requestOperationalInfo(bool loop = false) = 0;
    virtual void requestOperatingTimes(bool loop = false) = 0;
    virtual void requestOperatingState(bool loop = false) = 0;
    virtual void requestBurningDuration(bool loop = false) = 0;
    virtual void requestStartCounters(bool loop = false) = 0;
    virtual void requestSubsystemsStatus(bool loop = false) = 0;
    virtual void requestFuelPrewarming(bool loop = false) = 0;

    virtual OperationalMeasurements getOperationalMeasurementsData() = 0;
    virtual FuelSettings getFuelSettingsData() = 0;
    virtual OnOffFlags getOnOffFlagsData() = 0;
    virtual StatusFlags getStatusFlagsData() = 0;
    virtual OperatingState getOperatingStateData() = 0;
    virtual SubsystemsStatus geTsubsystemsStatusData() = 0;
    virtual OperatingTimes getOperatingTimesData() = 0;
    virtual FuelPrewarming getFuelPrewarmingData() = 0;
    virtual BurningDuration getBurningDurationData() = 0;
    virtual StartCounters getStartCountersData() = 0;
};