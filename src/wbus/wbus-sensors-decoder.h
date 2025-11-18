#ifndef WBUS_SENSORS_DECODER_H
#define WBUS_SENSORS_DECODER_H

#include <Arduino.h>

// Структуры данных (те же самые)
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

struct SubsystemsStatus
{
    uint8_t glowPlugPower;        // Мощность свечи накаливания в процентах * 2
    uint8_t fuelPumpFrequency;    // Частота топливного насоса в Гц * 2
    uint8_t combustionFanPower;   // Мощность вентилятора горения в процентах * 2
    uint8_t unknownByte3;         // Неизвестный байт
    uint8_t circulationPumpPower; // Мощность циркуляционного насоса в процентах * 2

    // Вычисленные значения
    float glowPlugPowerPercent;        // Мощность свечи в %
    float fuelPumpFrequencyHz;         // Частота ТН в Гц
    float combustionFanPowerPercent;   // Мощность вентилятора в %
    float circulationPumpPowerPercent; // Мощность циркуляционного насоса в %

    String statusSummary; // Сводка статуса
};

class WBusSensorsDecoder
{
private:
    String getStateName(uint8_t stateCode);
    String getStateDescription(uint8_t stateCode);
    String decodeDeviceStateFlags(uint8_t flags);
    String determineFuelTypeName(uint8_t fuelType);
    String buildActiveComponentsString(const OnOffFlags &flags);
    String buildStatusSummaryString(const StatusFlags &flags);
    String determineOperationMode(const StatusFlags &flags);

    // Вспомогательные методы
    String determineVentilationDescription(uint8_t ventFactor);
    bool validatePacketStructure(const String &response, uint8_t expectedCommand, uint8_t expectedIndex, int minLength);
    String buildSubsystemsSummaryString(const SubsystemsStatus &status);

public:
    OperationalMeasurements decodeOperationalInfo(const String &response);
    FuelSettings decodeFuelSettings(const String &response);
    OnOffFlags decodeOnOffFlags(const String &response);
    StatusFlags decodeStatusFlags(const String &response);
    OperatingState decodeOperatingState(const String &response);
    SubsystemsStatus decodeSubsystemsStatus(const String &response);
};

extern WBusSensorsDecoder wBusSensorsDecoder;

#endif // WBUS_SENSORS_DECODER_H