#ifndef WBUS_SENSORS_H
#define WBUS_SENSORS_H

#include <Arduino.h>

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

struct OnOffFlags
{
    bool combustionAirFan;   // Вентилятор горения (CAF)
    bool glowPlug;           // Свеча накаливания / искровой разрядник
    bool fuelPump;           // Топливный насос
    bool circulationPump;    // Циркуляционный насос
    bool vehicleFanRelay;    // Реле вентилятора автомобиля
    bool nozzleStockHeating; // Подогрев форсунки
    bool flameIndicator;     // Индикатор пламени
    String activeComponents; // Строка активных компонентов
};

struct StatusFlags
{
    // Байт 0
    bool mainSwitch;              // Главный выключатель
    bool supplementalHeatRequest; // Запрос дополнительного нагрева
    bool parkingHeatRequest;      // Запрос паркинг-нагрева
    bool ventilationRequest;      // Запрос вентиляции

    // Байт 1
    bool summerMode;      // Летний режим
    bool externalControl; // Внешнее управление

    // Байт 2
    bool generatorSignal; // Сигнал генератора D+

    // Байт 3
    bool boostMode;      // Boost режим
    bool auxiliaryDrive; // Вспомогательный привод

    // Байт 4
    bool ignitionSignal; // Сигнал зажигания (T15)

    String statusSummary; // Сводка статуса
    String operationMode; // Режим работы
};

struct OperatingState
{
    uint8_t stateCode;        // Код состояния (0x00-0x62)
    uint8_t stateNumber;      // Номер состояния
    uint8_t deviceStateFlags; // Флаги состояния устройства
    String stateName;         // Название состояния
    String stateDescription;  // Описание состояния
    String deviceStateInfo;   // Информация о состоянии устройства
};

class WebastoSensors
{
private:
    OperationalMeasurements operationalMeasurements;
    FuelSettings fuelSettings;
    OnOffFlags onOffFlags;
    StatusFlags statusFlags;
    OperatingState operatingState;

    void handleOperationalInfoResponse(bool success, String cmd, String response);
    void handleFuelSettingsResponse(bool success, String cmd, String response);
    void handleOnOffFlagsResponse(bool success, String cmd, String response);
    void handleStatusFlagsResponse(bool success, String cmd, String response);
    void handleOperatingStateResponse(bool success, String cmd, String response);

    String getStateName(uint8_t stateCode);
    String getStateDescription(uint8_t stateCode);
    String decodeDeviceStateFlags(uint8_t flags);

public:
    void getOperationalInfo();
    void getFuelSettings();
    void getOnOffFlags();
    void getStatusFlags();
    void getOperatingState();

    OperationalMeasurements getCurrentMeasurements() { return operationalMeasurements; }
    FuelSettings getFuelSettingsData() { return fuelSettings; }
    OnOffFlags getOnOffFlagsData() { return onOffFlags; }
    StatusFlags getStatusFlagsData() { return statusFlags; }
    OperatingState getOperatingStateData() { return operatingState; }
};

extern WebastoSensors webastoSensors;

#endif // WBUS_SENSORS_H