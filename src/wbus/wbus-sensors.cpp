#include "wbus-sensors.h"
#include "wbus-queue.h"
#include "wbus.constants.h"

WebastoSensors webastoSensors;

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
  Serial.println("   Сводка:             " + subsystemsStatus.statusSummary);
  Serial.println("   📊 Детальные параметры:");
  Serial.printf("      Свеча накаливания:  %5.1f %%\n", subsystemsStatus.glowPlugPowerPercent);
  Serial.printf("      Топливный насос:    %5.1f Гц\n", subsystemsStatus.fuelPumpFrequencyHz);
  Serial.printf("      Вентилятор горения: %5.1f %%\n", subsystemsStatus.combustionFanPowerPercent);
  Serial.printf("      Циркуляционный насос:%5.1f %%\n", subsystemsStatus.circulationPumpPowerPercent);
  Serial.println();
  Serial.println("═══════════════════════════════════════════════════════════");
  Serial.println();
}

// =============================================================================
// ОБРАБОТЧИКИ ОТВЕТОВ (УПРОЩЕННЫЕ)
// =============================================================================

void WebastoSensors::handleOperationalInfoResponse(bool success, String cmd, String response)
{
  if (success)
  {
    operationalMeasurements = wBusSensorsDecoder.decodeOperationalInfo(response);
  }
}

void WebastoSensors::handleFuelSettingsResponse(bool success, String cmd, String response)
{
  if (success)
  {
    fuelSettings = wBusSensorsDecoder.decodeFuelSettings(response);
  }
}

void WebastoSensors::handleOnOffFlagsResponse(bool success, String cmd, String response)
{
  if (success)
  {
    onOffFlags = wBusSensorsDecoder.decodeOnOffFlags(response);
  }
}

void WebastoSensors::handleStatusFlagsResponse(bool success, String cmd, String response)
{
  if (success)
  {
    statusFlags = wBusSensorsDecoder.decodeStatusFlags(response);
  }
}

void WebastoSensors::handleOperatingStateResponse(bool success, String cmd, String response)
{
  if (success)
  {
    operatingState = wBusSensorsDecoder.decodeOperatingState(response);
  }
}

void WebastoSensors::handleSubsystemsStatusResponse(bool success, String cmd, String response)
{
  if (success)
  {
    subsystemsStatus = wBusSensorsDecoder.decodeSubsystemsStatus(response);
  }
}

// =============================================================================
// ПУБЛИЧНЫЕ МЕТОДЫ
// =============================================================================

void WebastoSensors::getOperationalInfo(bool loop)
{
  wbusQueue.add(CMD_READ_SENSOR_OPERATIONAL, [this](bool success, String cmd, String response)
                { this->handleOperationalInfoResponse(success, cmd, response); }, loop);
}

void WebastoSensors::getFuelSettings(bool loop)
{
  wbusQueue.add(CMD_READ_SENSOR_FUEL_SETTINGS, [this](bool success, String cmd, String response)
                { this->handleFuelSettingsResponse(success, cmd, response); }, loop);
}

void WebastoSensors::getOnOffFlags(bool loop)
{
  wbusQueue.add(CMD_READ_SENSOR_ON_OFF_FLAGS, [this](bool success, String cmd, String response)
                { this->handleOnOffFlagsResponse(success, cmd, response); }, loop);
}

void WebastoSensors::getStatusFlags(bool loop)
{
  wbusQueue.add(CMD_READ_SENSOR_STATUS_FLAGS, [this](bool success, String cmd, String response)
                { this->handleStatusFlagsResponse(success, cmd, response); }, loop);
}

void WebastoSensors::getOperatingState(bool loop)
{
  wbusQueue.add(CMD_READ_SENSOR_OPERATING_STATE, [this](bool success, String cmd, String response)
                { this->handleOperatingStateResponse(success, cmd, response); }, loop);
}

void WebastoSensors::getSubsystemsStatus(bool loop)
{
  wbusQueue.add(CMD_READ_SENSOR_SUBSYSTEMS_STATUS, [this](bool success, String cmd, String response)
                { this->handleSubsystemsStatusResponse(success, cmd, response); }, loop);
}

void WebastoSensors::getAllSensorData(bool loop)
{
  getOperationalInfo(loop);
  if (!loop)
  {
   getFuelSettings(loop);
  }
  getOnOffFlags(loop);
  getStatusFlags(loop);
  getOperatingState(loop);
  getSubsystemsStatus(loop);
}

void WebastoSensors::stopContinuousMonitoring()
{
  wbusQueue.removeCommand(CMD_READ_SENSOR_OPERATIONAL);
  wbusQueue.removeCommand(CMD_READ_SENSOR_ON_OFF_FLAGS);
  wbusQueue.removeCommand(CMD_READ_SENSOR_STATUS_FLAGS);
  wbusQueue.removeCommand(CMD_READ_SENSOR_OPERATING_STATE);
  wbusQueue.removeCommand(CMD_READ_SENSOR_SUBSYSTEMS_STATUS);
}

void WebastoSensors::setLoopInterval(unsigned long interval)
{
  wbusQueue.setProcessDelay(interval);
  Serial.println("⏱️ Интервал опроса установлен: " + String(interval) + "мс");
}