#include "wbus-sensors.h"
#include "wbus-queue.h"
#include "wbus.constants.h"

WebastoSensors webastoSensors;

// =============================================================================
// ОБРАБОТЧИКИ ОТВЕТОВ (УПРОЩЕННЫЕ)
// =============================================================================

OperationalMeasurements *WebastoSensors::handleOperationalInfoResponse(String rx)
{
  if (!rx.isEmpty())
  {
    operationalMeasurements = wBusSensorsDecoder.decodeOperationalInfo(rx);

    return &operationalMeasurements;
  }

  return nullptr;
}

FuelSettings *WebastoSensors::handleFuelSettingsResponse(String rx)
{
  if (!rx.isEmpty())
  {
    fuelSettings = wBusSensorsDecoder.decodeFuelSettings(rx);

    return &fuelSettings;
  }

  return nullptr;
}

OnOffFlags *WebastoSensors::handleOnOffFlagsResponse(String rx)
{
  if (!rx.isEmpty())
  {
    onOffFlags = wBusSensorsDecoder.decodeOnOffFlags(rx);

    return &onOffFlags;
  }

  return nullptr;
}

StatusFlags *WebastoSensors::handleStatusFlagsResponse(String rx)
{
  if (!rx.isEmpty())
  {
    statusFlags = wBusSensorsDecoder.decodeStatusFlags(rx);

    return &statusFlags;
  }

  return nullptr;
}

OperatingState *WebastoSensors::handleOperatingStateResponse(String rx)
{
  if (!rx.isEmpty())
  {
    operatingState = wBusSensorsDecoder.decodeOperatingState(rx);

    return &operatingState;
  }

  return nullptr;
}

SubsystemsStatus *WebastoSensors::handleSubsystemsStatusResponse(String rx)
{
  if (!rx.isEmpty())
  {
    subsystemsStatus = wBusSensorsDecoder.decodeSubsystemsStatus(rx);

    return &subsystemsStatus;
  }

  return nullptr;
}

// =============================================================================
// ПУБЛИЧНЫЕ МЕТОДЫ
// =============================================================================

void WebastoSensors::getOperationalInfo(bool loop, std::function<void(String, String, OperationalMeasurements *)> callback)
{
  wbusQueue.add(CMD_READ_SENSOR_OPERATIONAL, [this, callback](String tx, String rx)
                {
      OperationalMeasurements* data = this -> handleOperationalInfoResponse(rx);
      if (callback != nullptr) {
        callback(tx, rx, data);
      } }, loop);
}

void WebastoSensors::getFuelSettings(bool loop, std::function<void(String, String, FuelSettings *)> callback)
{
  wbusQueue.add(CMD_READ_SENSOR_FUEL_SETTINGS, [this, callback](String tx, String rx)
                {
    FuelSettings* data = this -> handleFuelSettingsResponse(rx);
    if (callback != nullptr) {
      callback(tx, rx, data);
    } }, loop);
}

void WebastoSensors::getOnOffFlags(bool loop, std::function<void(String, String, OnOffFlags *)> callback)
{
  wbusQueue.add(CMD_READ_SENSOR_ON_OFF_FLAGS, [this, callback](String tx, String rx)
                {
    OnOffFlags* data = this -> handleOnOffFlagsResponse(rx);
    if (callback != nullptr) {
      callback(tx, rx, data);
    } }, loop);
}

void WebastoSensors::getStatusFlags(bool loop, std::function<void(String, String, StatusFlags *)> callback)
{
  wbusQueue.add(CMD_READ_SENSOR_STATUS_FLAGS, [this, callback](String tx, String rx)
                {
    StatusFlags* data = this -> handleStatusFlagsResponse(rx);
    if (callback != nullptr) {
      callback(tx, rx, data);
    } }, loop);
}

void WebastoSensors::getOperatingState(bool loop, std::function<void(String, String, OperatingState *)> callback)
{
  wbusQueue.add(CMD_READ_SENSOR_OPERATING_STATE, [this, callback](String tx, String rx)
                {
    OperatingState* data = this -> handleOperatingStateResponse(rx);
    if (callback != nullptr) {
      callback(tx, rx, data);
    } }, loop);
}

void WebastoSensors::getSubsystemsStatus(bool loop, std::function<void(String, String, SubsystemsStatus *)> callback)
{
  wbusQueue.add(CMD_READ_SENSOR_SUBSYSTEMS_STATUS, [this, callback](String tx, String rx)
                {
    SubsystemsStatus* data = this -> handleSubsystemsStatusResponse(rx);
    if (callback != nullptr) {
      callback(tx, rx, data);
    } }, loop);
}

void WebastoSensors::stopMonitoring()
{
  wbusQueue.removeCommand(CMD_READ_SENSOR_OPERATIONAL);
  wbusQueue.removeCommand(CMD_READ_SENSOR_FUEL_SETTINGS);
  wbusQueue.removeCommand(CMD_READ_SENSOR_ON_OFF_FLAGS);
  wbusQueue.removeCommand(CMD_READ_SENSOR_STATUS_FLAGS);
  wbusQueue.removeCommand(CMD_READ_SENSOR_OPERATING_STATE);
  wbusQueue.removeCommand(CMD_READ_SENSOR_SUBSYSTEMS_STATUS);
}

void WebastoSensors::clear()
{
  operationalMeasurements = OperationalMeasurements{};
  fuelSettings = FuelSettings{};
  onOffFlags = OnOffFlags{};
  statusFlags = StatusFlags{};
  operatingState = OperatingState{};
  subsystemsStatus = SubsystemsStatus{};
}

// =============================================================================
// ФУНКЦИИ ФОРМИРОВАНИЯ JSON
// =============================================================================

String WebastoSensors::createJsonOperationalInfo(const OperationalMeasurements &data)
{
  // "operational_measurements"
  DynamicJsonDocument doc(1024);

  doc["temperature"] = data.temperature;
  doc["voltage"] = data.voltage;
  doc["heating_power"] = data.heatingPower;
  doc["flame_resistance"] = data.flameResistance;
  doc["flame_detected"] = data.flameDetected;

  String json;
  serializeJson(doc, json);
  return json;
}

String WebastoSensors::createJsonOperationalInfo()
{
  return createJsonOperationalInfo(operationalMeasurements);
}

String WebastoSensors::createJsonFuelSettings(const FuelSettings &data)
{
  // "fuel_settings"
  DynamicJsonDocument doc(1024);

  doc["fuel_type"] = data.fuelType;
  doc["fuel_type_name"] = data.fuelTypeName;
  doc["max_heating_time"] = data.maxHeatingTime;
  doc["ventilation_factor"] = data.ventilationFactor;

  String json;
  serializeJson(doc, json);
  return json;
}

String WebastoSensors::createJsonFuelSettings()
{
  return createJsonFuelSettings(fuelSettings);
}

String WebastoSensors::createJsonOnOffFlags(const OnOffFlags &data)
{
  // "on_off_flags"
  DynamicJsonDocument doc(1024);

  doc["combustion_air_fan"] = data.combustionAirFan;
  doc["glow_plug"] = data.glowPlug;
  doc["fuel_pump"] = data.fuelPump;
  doc["circulation_pump"] = data.circulationPump;
  doc["vehicle_fan_relay"] = data.vehicleFanRelay;
  doc["nozzle_stock_heating"] = data.nozzleStockHeating;
  doc["flame_indicator"] = data.flameIndicator;
  doc["active_components"] = data.activeComponents;

  String json;
  serializeJson(doc, json);
  return json;
}

String WebastoSensors::createJsonOnOffFlags()
{
  return createJsonOnOffFlags(onOffFlags);
}

String WebastoSensors::createJsonStatusFlags(const StatusFlags &data)
{
  // "status_flags"
  DynamicJsonDocument doc(2048);

  // Основные флаги
  doc["main_switch"] = data.mainSwitch;
  doc["supplemental_heat_request"] = data.supplementalHeatRequest;
  doc["parking_heat_request"] = data.parkingHeatRequest;
  doc["ventilation_request"] = data.ventilationRequest;
  doc["summer_mode"] = data.summerMode;
  doc["external_control"] = data.externalControl;
  doc["generator_signal"] = data.generatorSignal;
  doc["boost_mode"] = data.boostMode;
  doc["auxiliary_drive"] = data.auxiliaryDrive;
  doc["ignition_signal"] = data.ignitionSignal;

  // Сводная информация
  doc["status_summary"] = data.statusSummary;
  doc["operation_mode"] = data.operationMode;

  String json;
  serializeJson(doc, json);
  return json;
}

String WebastoSensors::createJsonStatusFlags()
{
  return createJsonStatusFlags(statusFlags);
}

String WebastoSensors::createJsonOperatingState(const OperatingState &data)
{
  // "operating_state"
  DynamicJsonDocument doc(2048);

  doc["state_code"] = data.stateCode;
  doc["state_number"] = data.stateNumber;
  doc["device_state_flags"] = data.deviceStateFlags;
  doc["state_name"] = data.stateName;
  doc["state_description"] = data.stateDescription;
  doc["device_state_info"] = data.deviceStateInfo;

  // Дополнительная информация в HEX формате
  doc["state_code_hex"] = "0x" + String(operatingState.stateCode, HEX);
  doc["device_state_flags_hex"] = "0x" + String(operatingState.deviceStateFlags, HEX);

  String json;
  serializeJson(doc, json);
  return json;
}

String WebastoSensors::createJsonOperatingState()
{
  return createJsonOperatingState(operatingState);
}

String WebastoSensors::createJsonSubsystemsStatus(const SubsystemsStatus &data)
{
  // "subsystems_status"
  DynamicJsonDocument doc(2048);

  // Сырые данные
  doc["glow_plug_power"] = data.glowPlugPower;
  doc["fuel_pump_frequency"] = data.fuelPumpFrequency;
  doc["combustion_fan_power"] = data.combustionFanPower;
  doc["circulation_pump_power"] = data.circulationPumpPower;
  doc["unknown_byte_3"] = data.unknownByte3;

  // Вычисленные значения
  doc["glow_plug_power_percent"] = data.glowPlugPowerPercent;
  doc["fuel_pump_frequency_hz"] = data.fuelPumpFrequencyHz;
  doc["combustion_fan_power_percent"] = data.combustionFanPowerPercent;
  doc["circulation_pump_power_percent"] = data.circulationPumpPowerPercent;

  String json;
  serializeJson(doc, json);
  return json;
}

String WebastoSensors::createJsonSubsystemsStatus()
{
  return createJsonSubsystemsStatus(subsystemsStatus);
}

// =============================================================================
// ФУНКЦИЯ ВЫВОДА В SERIAL
// =============================================================================

void WebastoSensors::printSensorData()
{
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