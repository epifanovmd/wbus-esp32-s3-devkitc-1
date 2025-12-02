// src/domain/Entities.h
#pragma once
#include <Arduino.h>
#include <map>
#include <vector>

enum class WebastoState
{
    OFF,
    READY,
    PARKING_HEAT,
    VENTILATION,
    SUPP_HEAT,
    BOOST,
    CIRC_PUMP,
    STARTUP,
    SHUTDOWN,
    ERROR
};

enum class ConnectionState
{
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    CONNECTION_FAILED
};

// Структуры для информации об устройстве
struct DecodedManufactureDate
{
    String dateString;
    uint8_t day;
    uint8_t month;
    uint16_t year;

    String toJson() const
    {
        String json = "{";
        json += "\"dateString\":\"" + dateString + "\",";
        json += "\"day\":" + String(day) + ",";
        json += "\"month\":" + String(month) + ",";
        json += "\"year\":" + String(year);
        json += "}";
        return json;
    }
};

struct DecodedWBusCode
{
    String codeString;
    String supportedFunctions;

    String toJson() const
    {
        String json = "{";
        json += "\"codeString\":\"" + codeString + "\",";
        json += "\"supportedFunctions\":\"" + supportedFunctions + "\"";
        json += "}";
        return json;
    }
};

// Структуры для сенсоров
struct OperationalMeasurements
{
    float temperature = 0;
    float voltage = 0;
    bool flameDetected = false;
    int heatingPower = 0;
    int flameResistance = 0;

    String toJson() const
    {
        String json = "{";
        json += "\"temperature\":" + String(temperature, 1) + ",";
        json += "\"voltage\":" + String(voltage, 1) + ",";
        json += "\"heatingPower\":" + String(heatingPower) + ",";
        json += "\"flameResistance\":" + String(flameResistance) + ",";
        json += "\"flameDetected\":" + String(flameDetected ? "true" : "false");
        json += "}";
        return json;
    }
};

struct FuelSettings
{
    uint8_t fuelType = 0;
    uint8_t maxHeatingTime = 0;
    uint8_t ventilationFactor = 0;
    String fuelTypeName = "";

    String toJson() const
    {
        String json = "{";
        json += "\"fuelType\":" + String(fuelType) + ",";
        json += "\"fuelTypeName\":\"" + fuelTypeName + "\",";
        json += "\"maxHeatingTime\":" + String(maxHeatingTime) + ",";
        json += "\"ventilationFactor\":" + String(ventilationFactor);
        json += "}";
        return json;
    }
};

struct OnOffFlags
{
    bool combustionAirFan = false;
    bool glowPlug = false;
    bool fuelPump = false;
    bool circulationPump = false;
    bool vehicleFanRelay = false;
    bool nozzleStockHeating = false;
    bool flameIndicator = false;

    String toJson() const
    {
        String json = "{";
        json += "\"combustionAirFan\":" + String(combustionAirFan ? "true" : "false") + ",";
        json += "\"glowPlug\":" + String(glowPlug ? "true" : "false") + ",";
        json += "\"fuelPump\":" + String(fuelPump ? "true" : "false") + ",";
        json += "\"circulationPump\":" + String(circulationPump ? "true" : "false") + ",";
        json += "\"vehicleFanRelay\":" + String(vehicleFanRelay ? "true" : "false") + ",";
        json += "\"nozzleStockHeating\":" + String(nozzleStockHeating ? "true" : "false") + ",";
        json += "\"flameIndicator\":" + String(flameIndicator ? "true" : "false") + ",";

        json += "}";
        return json;
    }
};

struct StatusFlags
{
    bool mainSwitch = false;
    bool supplementalHeatRequest = false;
    bool parkingHeatRequest = false;
    bool ventilationRequest = false;
    bool summerMode = false;
    bool externalControl = false;
    bool generatorSignal = false;
    bool boostMode = false;
    bool auxiliaryDrive = false;
    bool ignitionSignal = false;

    String toJson() const
    {
        String json = "{";
        json += "\"mainSwitch\":" + String(mainSwitch ? "true" : "false") + ",";
        json += "\"parkingHeatRequest\":" + String(supplementalHeatRequest ? "true" : "false") + ",";
        json += "\"parkingHeatRequest\":" + String(parkingHeatRequest ? "true" : "false") + ",";
        json += "\"ventilationRequest\":" + String(ventilationRequest ? "true" : "false") + ",";
        json += "\"summerMode\":" + String(summerMode ? "true" : "false") + ",";
        json += "\"externalControl\":" + String(externalControl ? "true" : "false") + ",";
        json += "\"generatorSignal\":" + String(generatorSignal ? "true" : "false") + ",";
        json += "\"boostMode\":" + String(boostMode ? "true" : "false") + ",";
        json += "\"auxiliaryDrive\":" + String(auxiliaryDrive ? "true" : "false") + ",";
        json += "\"ignitionSignal\":" + String(ignitionSignal ? "true" : "false") + ",";
        json += "}";
        return json;
    }
};

struct OperatingState
{
    uint8_t stateCode = 0;
    uint8_t stateNumber = 0;
    uint8_t deviceStateFlags = 0;
    String stateName = "";
    String stateDescription = "";
    String deviceStateInfo = "";

    String toJson() const
    {
        String json = "{";
        json += "\"stateCode\":" + String(stateCode) + ",";
        json += "\"stateNumber\":" + String(stateNumber) + ",";
        json += "\"deviceStateFlags\":" + String(deviceStateFlags) + ",";
        json += "\"stateName\":\"" + stateName + "\",";
        json += "\"stateDescription\":\"" + stateDescription + "\",";
        json += "\"deviceStateInfo\":\"" + deviceStateInfo + "\",";
        json += "\"stateCodeHex\":\"0x" + String(stateCode, HEX) + "\",";
        json += "\"deviceStateFlagsHex\":\"0x" + String(deviceStateFlags, HEX) + "\"";
        json += "}";
        return json;
    }
};

struct SubsystemsStatus
{
    uint8_t glowPlugPower = 0;
    uint8_t fuelPumpFrequency = 0;
    uint8_t combustionFanPower = 0;
    uint8_t unknownByte3 = 0;
    uint8_t circulationPumpPower = 0;

    float glowPlugPowerPercent = 0;
    float fuelPumpFrequencyHz = 0;
    float combustionFanPowerPercent = 0;
    float circulationPumpPowerPercent = 0;

    String toJson() const
    {
        String json = "{";
        json += "\"glowPlugPower\":" + String(glowPlugPower) + ",";
        json += "\"fuelPumpFrequency\":" + String(fuelPumpFrequency) + ",";
        json += "\"combustionFanPower\":" + String(combustionFanPower) + ",";
        json += "\"circulationPumpPower\":" + String(circulationPumpPower) + ",";
        json += "\"unknownByte3\":" + String(unknownByte3) + ",";
        json += "\"glowPlugPowerPercent\":" + String(glowPlugPowerPercent, 1) + ",";
        json += "\"fuelPumpFrequencyHz\":" + String(fuelPumpFrequencyHz, 1) + ",";
        json += "\"combustionFanPowerPercent\":" + String(combustionFanPowerPercent, 1) + ",";
        json += "\"circulationPumpPowerPercent\":" + String(circulationPumpPowerPercent, 1);
        json += "}";
        return json;
    }
};

// Структуры для ошибок
struct WebastoError
{
    uint8_t code;
    String description;
    uint8_t counter;
    String hexCode;

    WebastoError(uint8_t errorCode = 0, uint8_t errorCounter = 0)
        : code(errorCode), counter(errorCounter)
    {
        hexCode = "0x";
        if (code < 0x10)
            hexCode += "0";
        hexCode += String(code, HEX);
    }

    String toJson() const
    {
        String json = "{";
        json += "\"code\":" + String(code) + ",";
        json += "\"hexCode\":\"" + hexCode + "\",";
        json += "\"description\":\"" + description + "\",";
        json += "\"counter\":" + String(counter);
        json += "}";
        return json;
    }
};

struct ErrorDetails {
    uint8_t errorCode;
    uint8_t statusFlags;
    uint8_t counter;
    uint16_t operatingState;
    uint8_t temperature;
    uint16_t voltage;
    uint16_t operatingHours;
    uint8_t operatingMinutes;
    String description;
    
    String toJson() const {
        String json = "{";
        json += "\"errorCode\":" + String(errorCode) + ",";
        json += "\"hexErrorCode\":\"0x" + String(errorCode, HEX) + "\",";
        json += "\"statusFlags\":" + String(statusFlags) + ",";
        json += "\"counter\":" + String(counter) + ",";
        json += "\"operatingState\":" + String(operatingState) + ",";
        json += "\"temperature\":" + String(temperature + 50) + ",";
        json += "\"voltage\":" + String(voltage / 1000.0, 2) + ",";
        json += "\"operatingHours\":" + String(operatingHours) + ",";
        json += "\"operatingMinutes\":" + String(operatingMinutes) + ",";
        json += "\"description\":\"" + description + "\"";
        json += "}";
        return json;
    }
};

struct ErrorCollection
{
    std::vector<WebastoError> errors;
    int errorCount = 0;

    void clear()
    {
        errors.clear();
        errorCount = 0;
    }

    void addError(const WebastoError &error)
    {
        errors.push_back(error);
        errorCount = errors.size();
    }

    bool isEmpty() const
    {
        return errors.empty();
    }

    String toJson() const
    {
        String json = "{\"count\":" + String(errorCount) + ",\"errors\":[";
        for (size_t i = 0; i < errors.size(); i++)
        {
            json += errors[i].toJson();
            if (i < errors.size() - 1)
                json += ",";
        }
        json += "]}";
        return json;
    }
};

struct FuelPrewarming
{
    uint16_t resistance;  // Сопротивление в миллиомах
    uint16_t power;       // Мощность в ваттах
    float resistanceOhms; // Сопротивление в омах
    bool isActive;        // Активен ли подогрев

    String toJson() const
    {
        String json = "{";
        json += "\"resistance\":" + String(resistance) + ",";
        json += "\"power\":" + String(power) + ",";
        json += "\"isActive\":" + String(isActive ? "true" : "false");
        json += "}";
        return json;
    }
};

struct OperatingTimes
{
    uint16_t workingHours;
    uint8_t workingMinutes;
    uint16_t operatingHours;
    uint8_t operatingMinutes;

    String toJson() const
    {
        String json = "{";
        json += "\"workingHours\":" + String(workingHours) + ",";
        json += "\"workingMinutes\":" + String(workingMinutes) + ",";
        json += "\"operatingHours\":" + String(operatingHours) + ",";
        json += "\"operatingMinutes\":" + String(operatingMinutes);
        json += "}";
        return json;
    }
};


struct PowerLevelStats {
    uint16_t hours;
    uint8_t minutes;
    
    String toString() const {
        return String(hours) + "h " + String(minutes) + "m";
    }
    
    String toJson() const {
        return "{\"hours\":" + String(hours) + ",\"minutes\":" + String(minutes) + "}";
    }
};

struct BurningDuration {
    PowerLevelStats shLow;       // SH 0-33%
    PowerLevelStats shMedium;    // SH 34-66%
    PowerLevelStats shHigh;      // SH 67-100%
    PowerLevelStats shBoost;     // SH >100%
    PowerLevelStats zhLow;       // ZH 0-33%
    PowerLevelStats zhMedium;    // ZH 34-66%
    PowerLevelStats zhHigh;      // ZH 67-100%
    PowerLevelStats zhBoost;     // ZH >100%
    
    bool isEmpty() const {
        return shLow.hours == 0 && shMedium.hours == 0 && shHigh.hours == 0 && shBoost.hours == 0 &&
               zhLow.hours == 0 && zhMedium.hours == 0 && zhHigh.hours == 0 && zhBoost.hours == 0;
    }
    
    String toJson() const {
        String json = "{";
        json += "\"shLow\":" + shLow.toJson() + ",";
        json += "\"shMedium\":" + shMedium.toJson() + ",";
        json += "\"shHigh\":" + shHigh.toJson() + ",";
        json += "\"shBoost\":" + shBoost.toJson() + ",";
        json += "\"zhLow\":" + zhLow.toJson() + ",";
        json += "\"zhMedium\":" + zhMedium.toJson() + ",";
        json += "\"zhHigh\":" + zhHigh.toJson() + ",";
        json += "\"zhBoost\":" + zhBoost.toJson();
        json += "}";
        return json;
    }
};

struct StartCounters {
    uint16_t shStarts;      // Supplemental Heating запусков
    uint16_t zhStarts;      // Parking Heating запусков  
    uint16_t totalStarts;   // Общие запуски или резерв
    
    String toJson() const {
        String json = "{";
        json += "\"shStarts\":" + String(shStarts) + ",";
        json += "\"zhStarts\":" + String(zhStarts) + ",";
        json += "\"totalStarts\":" + String(totalStarts);
        json += "}";
        return json;
    }
};

// Основной статус системы
struct HeaterStatus
{
    WebastoState state = WebastoState::OFF;
    ConnectionState connection = ConnectionState::DISCONNECTED;

    bool isConnected() const { return connection == ConnectionState::CONNECTED; }

    static String getStateName(WebastoState state)
    {
        switch (state)
        {
        case WebastoState::OFF:
            return "OFF";
        case WebastoState::READY:
            return "READY";
        case WebastoState::PARKING_HEAT:
            return "PARKING_HEAT";
        case WebastoState::VENTILATION:
            return "VENTILATION";
        case WebastoState::SUPP_HEAT:
            return "SUPP_HEAT";
        case WebastoState::BOOST:
            return "BOOST";
        case WebastoState::CIRC_PUMP:
            return "CIRC_PUMP";
        case WebastoState::STARTUP:
            return "STARTUP";
        case WebastoState::SHUTDOWN:
            return "SHUTDOWN";
        case WebastoState::ERROR:
            return "ERROR";
        default:
            return "OFF";
        }
    }

    String getStateName() const
    {
        return getStateName(state);
    }

    static String getConnectionName(ConnectionState connection)
    {
        switch (connection)
        {
        case ConnectionState::DISCONNECTED:
            return "DISCONNECTED";
        case ConnectionState::CONNECTING:
            return "CONNECTING";
        case ConnectionState::CONNECTED:
            return "CONNECTED";
        case ConnectionState::CONNECTION_FAILED:
            return "CONNECTION_FAILED";
        default:
            return "DISCONNECTED";
        }
    }

    String getConnectionName() const
    {
        return getConnectionName(connection);
    }

    String toJson() const
    {
        String json = "{";
        json += "\"heaterState\":\"" + getStateName() + "\",";
        json += "\"connectionState\":\"" + getConnectionName() + "\",";
        json += "\"isConnected\":" + String(isConnected() ? "true" : "false") + ",";
        json += "}";
        return json;
    }
};