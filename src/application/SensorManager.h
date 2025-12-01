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

    OperationalMeasurements operationalMeasurements;
    OnOffFlags onOffFlags;
    StatusFlags statusFlags;
    OperatingState operatingState;
    SubsystemsStatus subsystemsStatus;
    FuelSettings fuelSettings;
    OperatingTimes operatingTimes;
    FuelPrewarming fuelPrewarming;
    BurningDuration burningDuration;
    StartCounters startCounters;

public:
    SensorManager(EventBus &bus, CommandManager &cmdManager)
        : eventBus(bus), commandManager(cmdManager)
    {
    }

    void requestAllSensorData(bool loop = false) override
    {
        requestOperationalInfo(loop);
        requestOnOffFlags(loop);
        requestStatusFlags(loop);
        requestOperatingState(loop);
        requestSubsystemsStatus(loop);
        requestFuelPrewarming(loop);
        requestOperatingTimes();
        requestBurningDuration();
        requestStartCounters();
        requestFuelSettings();
    }

    void requestOperationalInfo(bool loop = false, std::function<void(String tx, String rx, OperationalMeasurements *measurements)> callback = nullptr) override
    {
        commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_OPERATIONAL), [this, callback](String tx, String rx)
                                  { handleOperationalInfoResponse(tx, rx, callback); }, loop);
    }

    void requestOnOffFlags(bool loop = false, std::function<void(String tx, String rx, OnOffFlags *onOff)> callback = nullptr) override
    {
        commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_ON_OFF_FLAGS), [this, callback](String tx, String rx)
                                  { handleOnOffFlagsResponse(tx, rx, callback); }, loop);
    }

    void requestStatusFlags(bool loop = false, std::function<void(String tx, String rx, StatusFlags *status)> callback = nullptr) override
    {
        commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_STATUS_FLAGS), [this, callback](String tx, String rx)
                                  { handleStatusFlagsResponse(tx, rx, callback); }, loop);
    }

    void requestOperatingState(bool loop = false, std::function<void(String tx, String rx, OperatingState *state)> callback = nullptr) override
    {
        commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_OPERATING_STATE), [this, callback](String tx, String rx)
                                  { handleOperatingStateResponse(tx, rx, callback); }, loop);
    }

    void requestSubsystemsStatus(bool loop = false, std::function<void(String tx, String rx, SubsystemsStatus *subsystems)> callback = nullptr) override
    {
        commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_SUBSYSTEMS_STATUS), [this, callback](String tx, String rx)
                                  { handleSubsystemsStatusResponse(tx, rx, callback); }, loop);
    }

    void requestFuelSettings(bool loop = false, std::function<void(String tx, String rx, FuelSettings *fuel)> callback = nullptr) override
    {
        commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_FUEL_SETTINGS), [this, callback](String tx, String rx)
                                  { handleFuelSettingsResponse(tx, rx, callback); }, loop);
    }

    void requestOperatingTimes(bool loop = false, std::function<void(String tx, String rx, OperatingTimes *operatingTimes)> callback = nullptr) override
    {
        commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_OPERATING_TIMES), [this, callback](String tx, String rx)
                                  { handleOperatingTimesResponse(tx, rx, callback); }, loop);
    }

    void requestFuelPrewarming(bool loop = false, std::function<void(String tx, String rx, FuelPrewarming *fuelPrewarming)> callback = nullptr) override
    {
        commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_FUEL_PREWARMING), [this, callback](String tx, String rx)
                                  { handleFuelPrewarmingResponse(tx, rx, callback); }, loop);
    }

    void requestBurningDuration(bool loop = false, std::function<void(String tx, String rx, BurningDuration *burningDuration)> callback = nullptr) override
    {
        commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_BURNING_DURATION), [this, callback](String tx, String rx)
                                  { handleBurningDurationResponse(tx, rx, callback); }, loop);
    }

    void requestStartCounters(bool loop = false, std::function<void(String, String, StartCounters *)> callback = nullptr) override
    {
        commandManager.addCommand(WBusCommandBuilder::createReadSensor(WBusCommandBuilder::SENSOR_START_COUNTERS), [this, callback](String tx, String rx)
                                  { handleStartCountersResponse(tx, rx, callback); }, loop);
    }

    // =========================================================================

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

    void handleOperatingTimesResponse(String tx, String rx, std::function<void(String, String, OperatingTimes *)> callback = nullptr)
    {
        if (!rx.isEmpty())
        {
            // нужно тестировать
            operatingTimes = WBusOperatingTimesDecoder::decode(rx);
            eventBus.publish<OperatingTimes>(EventType::SENSOR_OPERATING_TIMES, operatingTimes);

            if (callback)
            {
                callback(tx, rx, &operatingTimes);
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

    OperationalMeasurements getOperationalMeasurementsData() override { return operationalMeasurements; }
    FuelSettings getFuelSettingsData() override { return fuelSettings; }
    OnOffFlags getOnOffFlagsData() override { return onOffFlags; }
    StatusFlags getStatusFlagsData() override { return statusFlags; }
    OperatingState geToperatingStateData() override { return operatingState; }
    SubsystemsStatus geTsubsystemsStatusData() override { return subsystemsStatus; }
    OperatingTimes getOperatingTimesData() override { return operatingTimes; }
    FuelPrewarming getFuelPrewarmingData() override { return fuelPrewarming; }
    BurningDuration getBurningDurationData() override { return burningDuration; }
    StartCounters getStartCountersData() override { return startCounters; }

    String getAllSensorsJson() const
    {
        String json = "{";
        json += "\"operational_measurements\":" + operationalMeasurements.toJson() + ",";
        json += "\"fuel_settings\":" + fuelSettings.toJson() + ",";
        json += "\"on_off_flags\":" + onOffFlags.toJson() + ",";
        json += "\"status_flags\":" + statusFlags.toJson() + ",";
        json += "\"operating_state\":" + operatingState.toJson() + ",";
        json += "\"subsystems_status\":" + subsystemsStatus.toJson() + ",";
        json += "\"operating_times\":" + operatingTimes.toJson() + ",";
        json += "\"fuel_prewarming\":" + fuelPrewarming.toJson() + ",";
        json += "\"burning_duration\":" + burningDuration.toJson();
        json += "\"start_counters\":" + startCounters.toJson();
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