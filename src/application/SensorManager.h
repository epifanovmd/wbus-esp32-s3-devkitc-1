// src/application/SensorManager.h
#pragma once
#include "../interfaces/ISensorManager.h"
#include "../core/EventBus.h"
#include "../infrastructure/protocol/WBusSensorsDecoder.h"
#include "../application/CommandManager.h"
#include "../domain/Events.h" 

class SensorManager : public ISensorManager {
private:
    EventBus& eventBus;
    CommandManager& commandManager;

    OperationalMeasurements operationalMeasurements;
    FuelSettings fuelSettings;
    OnOffFlags onOffFlags;
    StatusFlags statusFlags;
    OperatingState operatingState;
    SubsystemsStatus subsystemsStatus;

public:
    SensorManager(EventBus& bus, CommandManager& cmdManager) 
    : eventBus(bus)
    , commandManager(cmdManager)
     {}
    
    void requestAllSensorData(bool loop = false) {
        requestOperationalInfo(loop);  
        requestOnOffFlags(loop); 
        requestStatusFlags(loop);   
        requestOperatingState(loop);
        requestSubsystemsStatus(loop); 
        requestFuelSettings(); // настройки топлива достаточно получить один раз
    }
    
    void requestOperationalInfo(bool loop = false, std::function<void(String tx, String rx, OperationalMeasurements* measurements)> callback = nullptr) override {
        commandManager.addCommand(WBusProtocol::CMD_READ_SENSOR_OPERATIONAL,
            [this](String tx, String rx) {
                if (!rx.isEmpty()) {
                    operationalMeasurements = WBusSensorsDecoder::decodeOperationalInfo(rx);
                    eventBus.publish<SensorDataUpdatedEvent>(EventType::SENSOR_DATA_UPDATED, {operationalMeasurements});
                    
                    // Отправляем в WebSocket
                    eventBus.publish(EventType::OPERATIONAL_DATA_UPDATED, "SensorManager");
                }
            });
    }
    
    void requestFuelSettings(bool loop = false, std::function<void(String tx, String rx, FuelSettings* fuel)> callback = nullptr) override {
        commandManager.addCommand(WBusProtocol::CMD_READ_SENSOR_FUEL_SETTINGS,
            [this](String tx, String rx) {
                if (!rx.isEmpty()) {
                    fuelSettings = WBusSensorsDecoder::decodeFuelSettings(rx);
                    eventBus.publish(EventType::FUEL_SETTINGS_UPDATED, "SensorManager");
                }
            });
    }
    
    void requestOnOffFlags(bool loop = false, std::function<void(String tx, String rx, OnOffFlags* onOff)> callback = nullptr) override {
        commandManager.addCommand(WBusProtocol::CMD_READ_SENSOR_ON_OFF_FLAGS,
            [this](String tx, String rx) {
                if (!rx.isEmpty()) {
                    onOffFlags = WBusSensorsDecoder::decodeOnOffFlags(rx);
                    eventBus.publish(EventType::ON_OFF_FLAGS_UPDATED, "SensorManager");
                }
            });
    }
    
    void requestStatusFlags(bool loop = false, std::function<void(String tx, String rx, StatusFlags* status)> callback = nullptr) override {
        commandManager.addCommand(WBusProtocol::CMD_READ_SENSOR_STATUS_FLAGS,
            [this](String tx, String rx) {
                if (!rx.isEmpty()) {
                    statusFlags = WBusSensorsDecoder::decodeStatusFlags(rx);
                    eventBus.publish(EventType::STATUS_FLAGS_UPDATED, "SensorManager");
                }
            });
    }
    
    void requestOperatingState(bool loop = false, std::function<void(String tx, String rx, OperatingState* state)> callback = nullptr) override {
        commandManager.addCommand(WBusProtocol::CMD_READ_SENSOR_OPERATING_STATE,
            [this](String tx, String rx) {
                if (!rx.isEmpty()) {
                    operatingState = WBusSensorsDecoder::decodeOperatingState(rx);
                    eventBus.publish(EventType::OPERATING_STATE_UPDATED, "SensorManager");
                }
            });
    }
    
    void requestSubsystemsStatus(bool loop = false, std::function<void(String tx, String rx, SubsystemsStatus* subsystems)> callback = nullptr) override {
        commandManager.addCommand(WBusProtocol::CMD_READ_SENSOR_SUBSYSTEMS_STATUS,
            [this](String tx, String rx) {
                if (!rx.isEmpty()) {
                    subsystemsStatus = WBusSensorsDecoder::decodeSubsystemsStatus(rx);
                    eventBus.publish(EventType::SUBSYSTEMS_STATUS_UPDATED, "SensorManager");
                }
            });
    }
    

    OperationalMeasurements getOperationalMeasurementsData() override { return operationalMeasurements; }
    FuelSettings getFuelSettingsData() override { return fuelSettings; }
    OnOffFlags getOnOffFlagsData() override { return onOffFlags; }
    StatusFlags getStatusFlagsData() override { return statusFlags; }
    OperatingState geToperatingStateData() override { return operatingState; }
    SubsystemsStatus geTsubsystemsStatusData() override { return subsystemsStatus; }


    String getAllSensorsJson() const {
        String json = "{";
        json += "\"operational_measurements\":" + operationalMeasurements.toJson() + ",";
        json += "\"fuel_settings\":" + fuelSettings.toJson() + ",";
        json += "\"on_off_flags\":" + onOffFlags.toJson() + ",";
        json += "\"status_flags\":" + statusFlags.toJson() + ",";
        json += "\"operating_state\":" + operatingState.toJson() + ",";
        json += "\"subsystems_status\":" + subsystemsStatus.toJson();
        json += "}";
        return json;
    }
    
    void printSensorData() const {
        Serial.println();
        Serial.println("═══════════════════════════════════════════════════════════");
        Serial.println("                📊 ДАННЫЕ СЕНСОРОВ WEBASTO                ");
        Serial.println("═══════════════════════════════════════════════════════════");

        // Операционные измерения
        Serial.println();
        Serial.println("🌡️  ОПЕРАЦИОННЫЕ ИЗМЕРЕНИЯ:");
        Serial.printf("   Температура:      %6.1f °C\n", operationalMeasurements.temperature);
        Serial.printf("   Напряжение:        %6.1f V\n", operationalMeasurements.voltage);
        Serial.printf("   Мощность:          %6d W\n", operationalMeasurements.heatingPower);
        Serial.printf("   Сопротивление:     %6d мОм\n", operationalMeasurements.flameResistance);
        Serial.printf("   Пламя:             %14s\n", operationalMeasurements.flameDetected ? "Обнаружено" : "Отсутствует");

        // Настройки топлива
        Serial.println();
        Serial.println("⛽ НАСТРОЙКИ ТОПЛИВА:");
        Serial.println("   Тип топлива:      " + fuelSettings.fuelTypeName);
        Serial.println("   Код типа:         0x" + String(fuelSettings.fuelType, HEX) + " (" + String(fuelSettings.fuelType, DEC) + ")");
        Serial.println("   Макс. время нагрева: " + String(fuelSettings.maxHeatingTime) + " минут");
        Serial.println("   Коэф. вентиляции: " + String(fuelSettings.ventilationFactor) + " мин");

        // Состояние подсистем
        Serial.println();
        Serial.println("🔧 СОСТОЯНИЕ ПОДСИСТЕМ:");
        Serial.println("   Активные компоненты: " + onOffFlags.activeComponents);
        Serial.println("   📋 Статус компонентов:");
        Serial.println("      " + String(onOffFlags.combustionAirFan ? "✅" : "❌") + " Вентилятор горения (CAF)");
        Serial.println("      " + String(onOffFlags.glowPlug ? "✅" : "❌") + " Свеча накаливания (GP)");
        Serial.println("      " + String(onOffFlags.fuelPump ? "✅" : "❌") + " Топливный насос (FP)");
        Serial.println("      " + String(onOffFlags.circulationPump ? "✅" : "❌") + " Циркуляционный насос (CP)");
        Serial.println("      " + String(onOffFlags.vehicleFanRelay ? "✅" : "❌") + " Реле вентилятора авто (VFR)");
        Serial.println("      " + String(onOffFlags.nozzleStockHeating ? "✅" : "❌") + " Подогрев форсунки (NSH)");
        Serial.println("      " + String(onOffFlags.flameIndicator ? "✅" : "❌") + " Индикатор пламени (FI)");

        // Статусные флаги
        Serial.println();
        Serial.println("📊 СТАТУСНЫЕ ФЛАГИ:");
        Serial.println("   Сводка статуса:    " + statusFlags.statusSummary);
        Serial.println("   Режим работы:      " + statusFlags.operationMode);
        Serial.println("   🎯 Основной статус:");
        Serial.println("      " + String(statusFlags.mainSwitch ? "✅" : "❌") + " Главный выключатель");
        Serial.println("      " + String(statusFlags.ignitionSignal ? "✅" : "❌") + " Зажигание (T15)");
        Serial.println("      " + String(statusFlags.generatorSignal ? "✅" : "❌") + " Генератор (D+)");
        Serial.println("      " + String(statusFlags.summerMode ? "✅" : "❌") + " Летний режим");
        Serial.println("      " + String(statusFlags.externalControl ? "✅" : "❌") + " Внешнее управление");
        Serial.println("   🔥 Запросы режимов:");
        Serial.println("      " + String(statusFlags.parkingHeatRequest ? "✅" : "❌") + " Паркинг-нагрев");
        Serial.println("      " + String(statusFlags.supplementalHeatRequest ? "✅" : "❌") + " Доп. нагрев");
        Serial.println("      " + String(statusFlags.ventilationRequest ? "✅" : "❌") + " Вентиляция");
        Serial.println("      " + String(statusFlags.boostMode ? "✅" : "❌") + " Boost режим");
        Serial.println("      " + String(statusFlags.auxiliaryDrive ? "✅" : "❌") + " Вспомогательный привод");

        // Состояние работы
        Serial.println();
        Serial.println("🔄 СОСТОЯНИЕ РАБОТЫ:");
        Serial.println("   Код состояния:     0x" + String(operatingState.stateCode, HEX) + " (" + String(operatingState.stateCode, DEC) + ")");
        Serial.println("   Название:          " + operatingState.stateName);
        Serial.println("   Описание:          " + operatingState.stateDescription);
        Serial.println("   Номер состояния:   " + String(operatingState.stateNumber));
        Serial.println("   Флаги устройства:  0x" + String(operatingState.deviceStateFlags, HEX) + " [" + operatingState.deviceStateInfo + "]");

        Serial.println();
        Serial.println("⚙️  СТАТУС ПОДСИСТЕМ:");
        Serial.println("   📊 Детальные параметры:");
        Serial.printf("      Свеча накаливания:  %5.1f %%\n", subsystemsStatus.glowPlugPowerPercent);
        Serial.printf("      Топливный насос:    %5.1f Гц\n", subsystemsStatus.fuelPumpFrequencyHz);
        Serial.printf("      Вентилятор горения: %5.1f %%\n", subsystemsStatus.combustionFanPowerPercent);
        Serial.printf("      Циркуляционный насос:%5.1f %%\n", subsystemsStatus.circulationPumpPowerPercent);
        
        Serial.println();
        Serial.println("═══════════════════════════════════════════════════════════");
        Serial.println();
    }
    
    void clear() {
        operationalMeasurements = OperationalMeasurements{};
        fuelSettings = FuelSettings{};
        onOffFlags = OnOffFlags{};
        statusFlags = StatusFlags{};
        operatingState = OperatingState{};
        subsystemsStatus = SubsystemsStatus{};
    }
};