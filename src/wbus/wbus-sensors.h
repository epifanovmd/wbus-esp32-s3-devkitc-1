#ifndef WBUS_SENSORS_H
#define WBUS_SENSORS_H

#include <Arduino.h>
#include "wbus/wbus-queue.h"
#include "wbus/wbus.constants.h"

struct OperationalMeasurements
{
    float temperature;   // °C (byte0 - 50 offset)
    float voltage;       // V (byte1-2, big endian)
    bool flameDetected;  // (byte3)
    int heatingPower;    // W (byte4-5, big endian)
    int flameResistance; // mΩ (byte6-7, big endian)
};

struct FuelSettings
{
    uint8_t fuelType;          // Тип топлива
    uint8_t maxHeatingTime;    // Максимальное время нагрева в минутах
    uint8_t ventilationFactor; // Коэффициент сокращения времени вентиляции
    String fuelTypeName;       // Название типа топлива
};

class WebastoSensors
{
private:
    OperationalMeasurements operationalMeasurements;
    FuelSettings fuelSettings;

    void handleOperationalInfoResponse(bool success, String cmd, String response);
    void handleFuelSettingsResponse(bool success, String cmd, String response);

public:
    void getOperationalInfo();
    void getFuelSettings();

    OperationalMeasurements getCurrentMeasurements() { return operationalMeasurements; }
    FuelSettings getFuelSettingsData() { return fuelSettings; }
};

extern WebastoSensors webastoSensors;

#endif // WBUS_SENSORS_H