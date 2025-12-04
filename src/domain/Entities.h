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
        json += "\"day\":" + (String(day || "N/A")    ) + ",";
        json += "\"month\":" + (String(month || "N/A")) + ",";
        json += "\"year\":" + (String(year || "N/A")  );
        json += "}";
        return json;
    }
};

struct WBusCodeFlags
{
    // Byte 0 flags
    // bool ZH;                   // 0x01: Unknown (ZH)
    bool simpleOnOffControl;  // 0x08: Simple on/off control
    bool parkingHeating;      // 0x10: Parking heating
    bool supplementalHeating; // 0x20: Supplemental heating
    bool ventilation;         // 0x40: Ventilation
    bool boostMode;           // 0x80: Boost mode

    // Byte 1 flags
    bool externalCirculationPumpControl; // 0x02: External circulation pump control
    bool combustionAirFan;               // 0x04: Combustion air fan (CAV)
    bool glowPlug;                       // 0x08: Glow Plug (flame detector)
    bool fuelPump;                       // 0x10: Fuel pump (FP)
    bool circulationPump;                // 0x20: Circulation pump (CP)
    bool vehicleFanRelay;                // 0x40: Vehicle fan relay (VFR)
    bool yellowLED;                      // 0x80: Yellow LED present

    // Byte 2 flags
    bool greenLED;                // 0x01: Green LED present
    bool sparkTransmitter;        // 0x02: Spark transmitter (implies no Glow plug)
    bool solenoidValve;           // 0x04: Solenoid valve present (coolant circuit switching)
    bool auxiliaryDriveIndicator; // 0x08: Auxiliary drive indicator
    bool generatorSignalDPlus;    // 0x10: Generator signal D+ present
    bool fanInRPM;                // 0x20: Combustion air fan level is in RPM instead of percent
    // bool ZH2;                     // 0x40: Unknown (ZH)
    // bool ZH3;                     // 0x80: Unknown (ZH)

    // Byte 3 flags
    bool CO2Calibration;     // 0x02: CO2 calibration
    bool operationIndicator; // 0x08: Operation indicator (OI)

    // Byte 4 flags
    // uint8_t ZH4;             // 0x0F: Unknown (ZH)
    bool powerInWatts; // 0x10: Heating energy is in watts (if not set, in percent divided by 2)
    // bool ZH5;                // 0x20: Unknown (ZH)
    bool flameIndicator;     // 0x40: Flame indicator (FI)
    bool nozzleStockHeating; // 0x80: Nozzle Stock heating

    // Byte 5 flags
    bool fuelPrewarmingReadable; // 0x80: Fuel prewarming resistance and power can be read
    bool temperatureThresholds;  // 0x40: Temperature thresholds available (command 0x50 index 0x11)
    bool ignitionFlag;           // 0x20: Ignition (T15) flag present

    // Byte 6 flags
    bool setValuesAvailable; // 0x02: Set value flame detector resistance (FW-SET),
                             //       set value combustion air fan revolutions (BG-SET),
                             //       set value output temperature (AT-SET)

    // Method to create a JSON string from all flags
    String toJson() const
    {
        String json = "{";

        // Byte 0
        // json += "\"ZH\":" + String(ZH ? "true" : "false") + ",";
        json += "\"simpleOnOffControl\":" + String(simpleOnOffControl ? "true" : "false") + ",";
        json += "\"parkingHeating\":" + String(parkingHeating ? "true" : "false") + ",";
        json += "\"supplementalHeating\":" + String(supplementalHeating ? "true" : "false") + ",";
        json += "\"ventilation\":" + String(ventilation ? "true" : "false") + ",";
        json += "\"boostMode\":" + String(boostMode ? "true" : "false") + ",";

        // Byte 1
        json += "\"externalCirculationPumpControl\":" + String(externalCirculationPumpControl ? "true" : "false") + ",";
        json += "\"combustionAirFan\":" + String(combustionAirFan ? "true" : "false") + ",";
        json += "\"glowPlug\":" + String(glowPlug ? "true" : "false") + ",";
        json += "\"fuelPump\":" + String(fuelPump ? "true" : "false") + ",";
        json += "\"circulationPump\":" + String(circulationPump ? "true" : "false") + ",";
        json += "\"vehicleFanRelay\":" + String(vehicleFanRelay ? "true" : "false") + ",";
        json += "\"yellowLED\":" + String(yellowLED ? "true" : "false") + ",";

        // Byte 2
        json += "\"greenLED\":" + String(greenLED ? "true" : "false") + ",";
        json += "\"sparkTransmitter\":" + String(sparkTransmitter ? "true" : "false") + ",";
        json += "\"solenoidValve\":" + String(solenoidValve ? "true" : "false") + ",";
        json += "\"auxiliaryDriveIndicator\":" + String(auxiliaryDriveIndicator ? "true" : "false") + ",";
        json += "\"generatorSignalDPlus\":" + String(generatorSignalDPlus ? "true" : "false") + ",";
        json += "\"fanInRPM\":" + String(fanInRPM ? "true" : "false") + ",";
        // json += "\"ZH2\":" + String(ZH2 ? "true" : "false") + ",";
        // json += "\"ZH3\":" + String(ZH3 ? "true" : "false") + ",";

        // Byte 3
        json += "\"CO2Calibration\":" + String(CO2Calibration ? "true" : "false") + ",";
        json += "\"operationIndicator\":" + String(operationIndicator ? "true" : "false") + ",";

        // Byte 4
        // json += "\"ZH4\":" + String(ZH4) + ",";
        json += "\"powerInWatts\":" + String(powerInWatts ? "true" : "false") + ",";
        // json += "\"ZH5\":" + String(ZH5 ? "true" : "false") + ",";
        json += "\"flameIndicator\":" + String(flameIndicator ? "true" : "false") + ",";
        json += "\"nozzleStockHeating\":" + String(nozzleStockHeating ? "true" : "false") + ",";

        // Byte 5
        json += "\"fuelPrewarmingReadable\":" + String(fuelPrewarmingReadable ? "true" : "false") + ",";
        json += "\"temperatureThresholds\":" + String(temperatureThresholds ? "true" : "false") + ",";
        json += "\"ignitionFlag\":" + String(ignitionFlag ? "true" : "false") + ",";

        // Byte 6
        json += "\"setValuesAvailable\":" + String(setValuesAvailable ? "true" : "false");

        json += "}";
        return json;
    }

    // Method to create human readable summary
    String getSummary() const
    {
        String summary = "";
        if (parkingHeating)
            summary += "Паркинг-нагрев, ";
        if (supplementalHeating)
            summary += "Доп. нагрев, ";
        if (ventilation)
            summary += "Вентиляция, ";
        if (boostMode)
            summary += "Boost, ";
        if (combustionAirFan)
            summary += "Вентилятор горения, ";
        if (glowPlug)
            summary += "Свеча накала, ";
        if (fuelPump)
            summary += "Топливный насос, ";
        if (circulationPump)
            summary += "Циркуляционный насос, ";
        if (vehicleFanRelay)
            summary += "Реле вентилятора авто, ";
        if (solenoidValve)
            summary += "Соленоидный клапан, ";
        if (CO2Calibration)
            summary += "Калибровка CO2, ";
        if (powerInWatts)
            summary += "Мощность в ваттах, ";
        if (flameIndicator)
            summary += "Индикатор пламени, ";
        if (fuelPrewarmingReadable)
            summary += "Подогрев топлива, ";
        if (temperatureThresholds)
            summary += "Темп. пороги, ";
        if (ignitionFlag)
            summary += "Флаг зажигания, ";
        if (setValuesAvailable)
            summary += "Уставки, ";

        // Remove trailing comma and space
        if (summary.length() > 2)
        {
            summary.remove(summary.length() - 2);
        }

        return summary.isEmpty() ? "N/A" : summary;
    }

    void clear()
    {
        // Byte 0
        // ZH = false;
        simpleOnOffControl = false;
        parkingHeating = false;
        supplementalHeating = false;
        ventilation = false;
        boostMode = false;

        // Byte 1
        externalCirculationPumpControl = false;
        combustionAirFan = false;
        glowPlug = false;
        fuelPump = false;
        circulationPump = false;
        vehicleFanRelay = false;
        yellowLED = false;

        // Byte 2
        greenLED = false;
        sparkTransmitter = false;
        solenoidValve = false;
        auxiliaryDriveIndicator = false;
        generatorSignalDPlus = false;
        fanInRPM = false;
        // ZH2 = false;
        // ZH3 = false;

        // Byte 3
        CO2Calibration = false;
        operationIndicator = false;

        // Byte 4
        // ZH4 = 0;
        powerInWatts = false;
        // ZH5 = false;
        flameIndicator = false;
        nozzleStockHeating = false;

        // Byte 5
        fuelPrewarmingReadable = false;
        temperatureThresholds = false;
        ignitionFlag = false;

        // Byte 6
        setValuesAvailable = false;
    }
};

struct DecodedWBusCode
{
    String codeString = "N/A";   // Raw HEX string of W-Bus code
    WBusCodeFlags flags; // All decoded flags

    String toJson() const
    {
        String json = "{";
        json += "\"codeString\":\"" + codeString + "\",";
        json += "\"flags\":" + flags.toJson() + ",";
        json += "\"summary\":\"" + flags.getSummary() + "\"";
        json += "}";
        return json;
    }

    void clear()
    {
        codeString = "";
        flags.clear();
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
    String fuelTypeName = "N/A";

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
        json += "\"flameIndicator\":" + String(flameIndicator ? "true" : "false");
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
        json += "\"supplementalHeatRequest\":" + String(supplementalHeatRequest ? "true" : "false") + ",";
        json += "\"parkingHeatRequest\":" + String(parkingHeatRequest ? "true" : "false") + ",";
        json += "\"ventilationRequest\":" + String(ventilationRequest ? "true" : "false") + ",";
        json += "\"summerMode\":" + String(summerMode ? "true" : "false") + ",";
        json += "\"externalControl\":" + String(externalControl ? "true" : "false") + ",";
        json += "\"generatorSignal\":" + String(generatorSignal ? "true" : "false") + ",";
        json += "\"boostMode\":" + String(boostMode ? "true" : "false") + ",";
        json += "\"auxiliaryDrive\":" + String(auxiliaryDrive ? "true" : "false") + ",";
        json += "\"ignitionSignal\":" + String(ignitionSignal ? "true" : "false");
        json += "}";
        return json;
    }
};

struct OperatingState
{
    String stateName = "N/A";
    uint8_t stateNumber = 0;
    String deviceStateFlags = "N/A";

    String toJson() const
    {
        String json = "{";
        json += "\"stateName\":\"" + stateName + "\",";
        json += "\"stateNumber\":" + String(stateNumber) + ",";
        json += "\"deviceStateFlags\":\"" + deviceStateFlags + "\"";
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
    String errorName;
    String errorDescription;
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
        json += "\"errorName\":\"" + errorName + "\",";
        json += "\"errorDescription\":\"" + errorDescription + "\",";
        json += "\"counter\":" + String(counter);
        json += "}";
        return json;
    }
};

struct ErrorDetails
{
    String status; // Текстовое описание статуса

    uint8_t counter; // Количество срабатываний

    String stateCode; // Код состояния работы (первый байт состояния)
    String stateName; // Название состояния ("Heater interlock permanent")

    float temperature; // Температура в градусах Цельсия
    float voltage;     // Напряжение в вольтах

    uint32_t operatingHours;  // Часы работы на момент ошибки
    uint8_t operatingMinutes; // Минуты работы на момент ошибки

    // Конструктор по умолчанию
    ErrorDetails() : temperature(0),
                     voltage(0.0f),
                     operatingHours(0),
                     operatingMinutes(0) {}

    String toJson() const
    {
        String json = "{";
        json += "\"status\":\"" + status + "\",";
        json += "\"stateCode\":\"" + stateCode + "\",";
        json += "\"stateName\":\"" + stateName + "\",";
        json += "\"counter\":" + String(counter - 1) + ",";
        json += "\"temperature\":" + String(temperature) + ",";
        json += "\"voltage\":" + String(voltage) + ",";
        json += "\"operatingTime\":{";
        json += "\"hours\":" + String(operatingHours) + ",";
        json += "\"minutes\":" + String(operatingMinutes) + "";
        json += "}";
        json += "}";

        return json;
    }

private:
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
    uint16_t resistance; // Сопротивление в миллиомах
    uint16_t power;      // Мощность в ваттах
    bool isActive;       // Активен ли подогрев

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
    uint16_t startCounter;

    String toJson() const
    {
        String json = "{";
        json += "\"workingHours\":" + String(workingHours) + ",";
        json += "\"workingMinutes\":" + String(workingMinutes) + ",";
        json += "\"operatingHours\":" + String(operatingHours) + ",";
        json += "\"operatingMinutes\":" + String(operatingMinutes) + ",";
        json += "\"startCounter\":" + String(startCounter);
        json += "}";
        return json;
    }
};

struct PowerLevelStats
{
    uint16_t hours;
    uint8_t minutes;

    String toString() const
    {
        return String(hours) + "h " + String(minutes) + "m";
    }

    String toJson() const
    {
        return "{\"hours\":" + String(hours) + ",\"minutes\":" + String(minutes) + "}";
    }
};

struct BurningDuration
{
    PowerLevelStats shLow;    // SH 0-33%
    PowerLevelStats shMedium; // SH 34-66%
    PowerLevelStats shHigh;   // SH 67-100%
    PowerLevelStats shBoost;  // SH >100%
    PowerLevelStats zhLow;    // ZH 0-33%
    PowerLevelStats zhMedium; // ZH 34-66%
    PowerLevelStats zhHigh;   // ZH 67-100%
    PowerLevelStats zhBoost;  // ZH >100%

    bool isEmpty() const
    {
        return shLow.hours == 0 && shMedium.hours == 0 && shHigh.hours == 0 && shBoost.hours == 0 &&
               zhLow.hours == 0 && zhMedium.hours == 0 && zhHigh.hours == 0 && zhBoost.hours == 0;
    }

    String toJson() const
    {
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

struct StartCounters
{
    uint16_t shStarts;    // Supplemental Heating запусков
    uint16_t zhStarts;    // Parking Heating запусков
    uint16_t totalStarts; // Общие запуски или резерв

    String toJson() const
    {
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