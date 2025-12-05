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

    void requestStatusFlags(bool loop = false) override
    {
        commandManager.addPriorityCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_STATUS_FLAGS), loop);
    }

    void requestOnOffFlags(bool loop = false) override
    {
        commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_ON_OFF_FLAGS), loop);
    }

    void requestFuelSettings(bool loop = false) override
    {
        commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_FUEL_SETTINGS), loop);
    }

    void requestOperationalInfo(bool loop = false) override
    {
        commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_OPERATIONAL), loop);
    }

    void requestOperatingTimes(bool loop = false) override
    {
        commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_OPERATING_TIMES), loop);
    }

    void requestOperatingState(bool loop = false) override
    {
        commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_OPERATING_STATE), loop);
    }

    void requestBurningDuration(bool loop = false) override
    {
        commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_BURNING_DURATION), loop);
    }

    // void requestWorkingDuration(bool loop = false) override
    // {
    //     commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_WORKING_DURATION), loop);
    // }

    void requestStartCounters(bool loop = false) override
    {
        commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_START_COUNTERS), loop);
    }

    void requestSubsystemsStatus(bool loop = false) override
    {
        commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_SUBSYSTEMS_STATUS), loop);
    }

    // void requestOtherDuration(bool loop = false) override
    // {
    //     commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_OTHER_DURATION), loop);
    // }

    // void requestTemperatureThresholds(bool loop = false) override
    // {
    //     commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_TEMPERATURE_THRESHOLDS), loop);
    // }

    // void requestVentilationDuration(bool loop = false) override
    // {
    //     commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_VENTILATION_DURATION), loop);
    // }

    void requestFuelPrewarming(bool loop = false) override
    {
        commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_FUEL_PREWARMING), loop);
    }

    // void requestSparkTransmission(bool loop = false) override
    // {
    //     commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_SPARK_TRANSMISSION), loop);
    // }

    // =========================================================================

    void handleStatusFlagsResponse(String tx, String rx)
    {
        if (!rx.isEmpty())
            statusFlags = WBusStatusFlagsDecoder::decode(rx);
        eventBus.publish<StatusFlags>(EventType::SENSOR_STATUS_FLAGS, statusFlags);
    }

    void handleOnOffFlagsResponse(String tx, String rx)
    {
        onOffFlags = WBusOnOffFlagsDecoder::decode(rx);
        eventBus.publish<OnOffFlags>(EventType::SENSOR_ON_OFF_FLAGS, onOffFlags);
    }

    void handleFuelSettingsResponse(String tx, String rx)
    {
        fuelSettings = WBusFuelSettingsDecoder::decode(rx);
        eventBus.publish<FuelSettings>(EventType::FUEL_SETTINGS, fuelSettings);
    }

    void handleOperationalInfoResponse(String tx, String rx)
    {
        operationalMeasurements = WBusOperationalInfoDecoder::decode(rx);
        eventBus.publish<OperationalMeasurements>(EventType::SENSOR_OPERATIONAL_INFO, operationalMeasurements);
    }

    void handleOperatingTimesResponse(String tx, String rx)
    {
        operatingTimes = WBusOperatingTimesDecoder::decode(rx);
        eventBus.publish<OperatingTimes>(EventType::SENSOR_OPERATING_TIMES, operatingTimes);
    }

    void handleOperatingStateResponse(String tx, String rx)
    {
        operatingState = WBusOperatingStateDecoder::decode(rx);
        eventBus.publish<OperatingState>(EventType::SENSOR_OPERATING_STATE, operatingState);
    }

    void handleBurningDurationResponse(String tx, String rx)
    {
        burningDuration = WBusBurningDurationDecoder::decode(rx);
        eventBus.publish<BurningDuration>(EventType::BURNING_DURATION_STATS, burningDuration);
    }

    void handleStartCountersResponse(String tx, String rx)
    {
        startCounters = WBusStartCountersDecoder::decode(rx);
        eventBus.publish<StartCounters>(EventType::START_COUNTERS, startCounters);
    }

    void handleSubsystemsStatusResponse(String tx, String rx)
    {
        subsystemsStatus = WBusSubSystemsDecoder::decode(rx);
        eventBus.publish<SubsystemsStatus>(EventType::SENSOR_SUBSYSTEM_STATE, subsystemsStatus);
    }

    void handleFuelPrewarmingResponse(String tx, String rx)
    {
        fuelPrewarming = WBusFuelPrewarmingDecoder::decode(rx);
        eventBus.publish<FuelPrewarming>(EventType::FUEL_PREWARMING, fuelPrewarming);
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
        json += "\"startCounters\":" + startCounters.toJson() + ",";
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