#pragma once
#include "../interfaces/ISensorManager.h"
#include "../core/EventBus.h"
#include "../infrastructure/protocol/WBusCommandBuilder.h"
#include "../infrastructure/protocol/WBusFuelSettingsDecoder.h"
#include "../infrastructure/protocol/WBusOnOffFlagsDecoder.h"
#include "../infrastructure/protocol/WBusOperatingStateDecoder.h"
#include "../infrastructure/protocol/WBusOperationalInfoDecoder.h"
#include "../infrastructure/protocol/WBusStatusFlagsDecoder.h"
#include "../infrastructure/protocol/WBusSubSystemsDecoder.h"
#include "../infrastructure/protocol/WBusOperatingTimesDecoder.h"
#include "../infrastructure/protocol/WBusFuelPrewarmingDecoder.h"
#include "../infrastructure/protocol/WBusBurningDurationDecoder.h"
#include "../infrastructure/protocol/WBusStartCountersDecoder.h"
#include "../application/CommandManager.h"
#include "../domain/Events.h"

class SensorManager : public ISensorManager
{
private:
    EventBus &eventBus;
    CommandManager &commandManager;

    StatusFlags statusFlags;
    OnOffFlags onOffFlags;
    FuelSettings fuelSettings;
    OperationalMeasurements operationalMeasurements;
    OperatingTimes operatingTimes;
    OperatingState operatingState;
    BurningDuration burningDuration;
    StartCounters startCounters;
    SubsystemsStatus subsystemsStatus;
    FuelPrewarming fuelPrewarming;

public:
    SensorManager(EventBus &bus, CommandManager &cmdManager)
        : eventBus(bus), commandManager(cmdManager)
    {
    }

    void requestAllSensorData(bool loop = false) override
    {
        requestStatusFlags(loop);
        requestOnOffFlags(loop);
        requestFuelSettings();
        requestOperationalInfo(loop);
        requestOperatingTimes();
        requestOperatingState(loop);
        requestBurningDuration();
        requestStartCounters();
        requestSubsystemsStatus(loop);
        requestFuelPrewarming(loop);
    }

    void requestStatusFlags(bool loop = false, std::function<void(String tx, String rx)> callback = nullptr) override
    {
        commandManager.addPriorityCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_STATUS_FLAGS), callback, loop);
    }

    void requestOnOffFlags(bool loop = false, std::function<void(String tx, String rx)> callback = nullptr) override
    {
        commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_ON_OFF_FLAGS), callback, loop);
    }

    void requestFuelSettings(bool loop = false, std::function<void(String tx, String rx)> callback = nullptr) override
    {
        commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_FUEL_SETTINGS), callback, loop);
    }

    void requestOperationalInfo(bool loop = false, std::function<void(String tx, String rx)> callback = nullptr) override
    {
        commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_OPERATIONAL), callback, loop);
    }

    void requestOperatingTimes(bool loop = false, std::function<void(String tx, String rx)> callback = nullptr) override
    {
        commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_OPERATING_TIMES), callback, loop);
    }

    void requestOperatingState(bool loop = false, std::function<void(String tx, String rx)> callback = nullptr) override
    {
        commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_OPERATING_STATE), callback, loop);
    }

    void requestBurningDuration(bool loop = false, std::function<void(String tx, String rx)> callback = nullptr) override
    {
        commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_BURNING_DURATION), callback, loop);
    }

    // void requestWorkingDuration(bool loop = false, std::function<void(String tx, String rx)> callback = nullptr) override
    // {
    //     commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_WORKING_DURATION), callback, loop);
    // }

    void requestStartCounters(bool loop = false, std::function<void(String, String)> callback = nullptr) override
    {
        commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_START_COUNTERS), callback, loop);
    }

    void requestSubsystemsStatus(bool loop = false, std::function<void(String tx, String rx)> callback = nullptr) override
    {
        commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_SUBSYSTEMS_STATUS), callback, loop);
    }

    // void requestOtherDuration(bool loop = false, std::function<void(String tx, String rx)> callback = nullptr) override
    // {
    //     commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_OTHER_DURATION), callback, loop);
    // }

    // void requestTemperatureThresholds(bool loop = false, std::function<void(String tx, String rx)> callback = nullptr) override
    // {
    //     commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_TEMPERATURE_THRESHOLDS), callback, loop);
    // }

    // void requestVentilationDuration(bool loop = false, std::function<void(String tx, String rx)> callback = nullptr) override
    // {
    //     commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_VENTILATION_DURATION), callback, loop);
    // }

    void requestFuelPrewarming(bool loop = false, std::function<void(String tx, String rx)> callback = nullptr) override
    {
        commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_FUEL_PREWARMING), callback, loop);
    }

    // void requestSparkTransmission(bool loop = false, std::function<void(String tx, String rx)> callback = nullptr) override
    // {
    //     commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_SPARK_TRANSMISSION), callback, loop);
    // }

    // =========================================================================

    void handleStatusFlagsResponse(String tx, String rx, std::function<void(String tx, String rx, StatusFlags *status)> callback = nullptr)
    {
        if (!rx.isEmpty())
        {
            statusFlags = WBusStatusFlagsDecoder::decode(rx);
            eventBus.publish<StatusFlags>(EventType::SENSOR_STATUS_FLAGS, statusFlags);

            if (callback)
            {
                callback(tx, rx, &statusFlags);
            }
        }
    }

    void handleOnOffFlagsResponse(String tx, String rx, std::function<void(String tx, String rx, OnOffFlags *onOff)> callback = nullptr)
    {
        if (!rx.isEmpty())
        {
            onOffFlags = WBusOnOffFlagsDecoder::decode(rx);
            eventBus.publish<OnOffFlags>(EventType::SENSOR_ON_OFF_FLAGS, onOffFlags);

            if (callback)
            {
                callback(tx, rx, &onOffFlags);
            }
        }
    }

    void handleFuelSettingsResponse(String tx, String rx, std::function<void(String tx, String rx, FuelSettings *fuel)> callback = nullptr)
    {
        if (!rx.isEmpty())
        {
            fuelSettings = WBusFuelSettingsDecoder::decode(rx);
            eventBus.publish<FuelSettings>(EventType::FUEL_SETTINGS, fuelSettings);

            if (callback)
            {
                callback(tx, rx, &fuelSettings);
            }
        }
    }

    void handleOperationalInfoResponse(String tx, String rx, std::function<void(String tx, String rx, OperationalMeasurements *measurements)> callback = nullptr)
    {
        if (!rx.isEmpty())
        {
            operationalMeasurements = WBusOperationalInfoDecoder::decode(rx);
            eventBus.publish<OperationalMeasurements>(EventType::SENSOR_OPERATIONAL_INFO, operationalMeasurements);

            if (callback)
            {
                callback(tx, rx, &operationalMeasurements);
            }
        }
    }

    void handleOperatingTimesResponse(String tx, String rx, std::function<void(String, String, OperatingTimes *)> callback = nullptr)
    {
        if (!rx.isEmpty())
        {
            operatingTimes = WBusOperatingTimesDecoder::decode(rx);
            eventBus.publish<OperatingTimes>(EventType::SENSOR_OPERATING_TIMES, operatingTimes);

            if (callback)
            {
                callback(tx, rx, &operatingTimes);
            }
        }
    }

    void handleOperatingStateResponse(String tx, String rx, std::function<void(String tx, String rx, OperatingState *state)> callback = nullptr)
    {
        if (!rx.isEmpty())
        {
            operatingState = WBusOperatingStateDecoder::decode(rx);
            eventBus.publish<OperatingState>(EventType::SENSOR_OPERATING_STATE, operatingState);

            if (callback)
            {
                callback(tx, rx, &operatingState);
            }
        }
    }

    void handleBurningDurationResponse(String tx, String rx, std::function<void(String, String, BurningDuration *)> callback = nullptr)
    {
        if (!rx.isEmpty())
        {
            burningDuration = WBusBurningDurationDecoder::decode(rx);
            eventBus.publish<BurningDuration>(EventType::BURNING_DURATION_STATS, burningDuration);

            if (callback)
            {
                callback(tx, rx, &burningDuration);
            }
        }
    }

    void handleStartCountersResponse(String tx, String rx, std::function<void(String, String, StartCounters *)> callback = nullptr)
    {
        if (!rx.isEmpty())
        {
            startCounters = WBusStartCountersDecoder::decode(rx);
            eventBus.publish<StartCounters>(EventType::START_COUNTERS, startCounters);

            if (callback)
            {
                callback(tx, rx, &startCounters);
            }
        }
    }

    void handleSubsystemsStatusResponse(String tx, String rx, std::function<void(String tx, String rx, SubsystemsStatus *subsystems)> callback = nullptr)
    {
        if (!rx.isEmpty())
        {
            subsystemsStatus = WBusSubSystemsDecoder::decode(rx);
            eventBus.publish<SubsystemsStatus>(EventType::SENSOR_SUBSYSTEM_STATE, subsystemsStatus);

            if (callback)
            {
                callback(tx, rx, &subsystemsStatus);
            }
        }
    }

    void handleFuelPrewarmingResponse(String tx, String rx, std::function<void(String, String, FuelPrewarming *)> callback = nullptr)
    {
        if (!rx.isEmpty())
        {
            fuelPrewarming = WBusFuelPrewarmingDecoder::decode(rx);
            eventBus.publish<FuelPrewarming>(EventType::FUEL_PREWARMING, fuelPrewarming);

            if (callback)
            {
                callback(tx, rx, &fuelPrewarming);
            }
        }
    }

    StatusFlags getStatusFlagsData() override { return statusFlags; }
    OnOffFlags getOnOffFlagsData() override { return onOffFlags; }
    FuelSettings getFuelSettingsData() override { return fuelSettings; }
    OperationalMeasurements getOperationalMeasurementsData() override { return operationalMeasurements; }
    OperatingTimes getOperatingTimesData() override { return operatingTimes; }
    OperatingState getOperatingStateData() override { return operatingState; }
    BurningDuration getBurningDurationData() override { return burningDuration; }
    StartCounters getStartCountersData() override { return startCounters; }
    SubsystemsStatus geTsubsystemsStatusData() override { return subsystemsStatus; }
    FuelPrewarming getFuelPrewarmingData() override { return fuelPrewarming; }

    String getAllSensorsJson() const
    {
        String json = "{";
        json += "\"statusFlags\":" + statusFlags.toJson() + ",";
        json += "\"onOffFlags\":" + onOffFlags.toJson() + ",";
        json += "\"fuelSettings\":" + fuelSettings.toJson() + ",";
        json += "\"operationalMeasurements\":" + operationalMeasurements.toJson() + ",";
        json += "\"operatingTimes\":" + operatingTimes.toJson() + ",";
        json += "\"operatingState\":" + operatingState.toJson() + ",";
        json += "\"burningDuration\":" + burningDuration.toJson() + ",";
        json += "\"startCounters\":" + startCounters.toJson();
        +",";
        json += "\"subsystemsStatus\":" + subsystemsStatus.toJson() + ",";
        json += "\"fuelPrewarming\":" + fuelPrewarming.toJson();
        json += "}";
        return json;
    }

    void clear()
    {
        operationalMeasurements = OperationalMeasurements{};
        fuelSettings = FuelSettings{};
        onOffFlags = OnOffFlags{};
        statusFlags = StatusFlags{};
        operatingState = OperatingState{};
        subsystemsStatus = SubsystemsStatus{};
        operatingTimes = OperatingTimes{};
        fuelPrewarming = FuelPrewarming{};
        burningDuration = BurningDuration{};
        startCounters = StartCounters{};
    }
};