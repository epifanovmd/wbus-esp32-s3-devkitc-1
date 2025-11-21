// src/domain/Events.h
#pragma once
#include "Entities.h"

struct SensorDataUpdatedEvent {
    OperationalMeasurements data;
    uint32_t sequenceNumber;
};

struct HeaterStateChangedEvent {
    WebastoState oldState;
    WebastoState newState;
    String reason;
};

struct ConnectionStateChangedEvent {
    ConnectionState oldState;
    ConnectionState newState;
    String reason;
};

struct CommandSentEvent {
    String command;
    String description;
};

struct CommandReceivedEvent {
    String command;
    String response;
    bool success;
};

// Новые события для полной функциональности
struct DeviceInfoUpdatedEvent {
    String deviceInfoJson;
};

struct ErrorsUpdatedEvent {
    String errorsJson;
};

struct OperationalDataUpdatedEvent {
    OperationalMeasurements data;
};

struct FuelSettingsUpdatedEvent {
    FuelSettings data;
};

struct OnOffFlagsUpdatedEvent {
    OnOffFlags data;
};

struct StatusFlagsUpdatedEvent {
    StatusFlags data;
};

struct OperatingStateUpdatedEvent {
    OperatingState data;
};

struct SubsystemsStatusUpdatedEvent {
    SubsystemsStatus data;
};