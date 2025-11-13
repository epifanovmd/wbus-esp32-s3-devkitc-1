#ifndef WBUSDECODERS_H
#define WBUSDECODERS_H

#include <Arduino.h>

// =============================================
// СТРУКТУРЫ ДЛЯ ДЕКОДИРОВАННЫХ ДАННЫХ
// =============================================

struct SensorStatusFlags {
    bool supplementalHeaterRequest; // 0x10
    bool mainSwitch;               // 0x01
    bool summerSeason;             // 0x01 (byte1)
    bool generatorSignal;          // 0x10 (byte2) 
    bool boostMode;                // 0x10 (byte3)
    bool auxiliaryDrive;           // 0x01 (byte3)
    bool ignition;                 // 0x01 (byte4)
};

struct OnOffFlags {
    bool combustionAirFan;    // 0x01
    bool glowPlug;           // 0x02  
    bool fuelPump;           // 0x04
    bool circulationPump;    // 0x08
    bool vehicleFanRelay;    // 0x10
    bool nozzleStockHeating; // 0x20
    bool flameIndicator;     // 0x40
};

struct OperationalMeasurements {
    float temperature;       // °C (byte0 - 50 offset)
    float voltage;           // V (byte1-2, big endian)
    bool flameDetected;      // (byte3)
    int heatingPower;        // W (byte4-5, big endian)
    int flameResistance;     // mΩ (byte6-7, big endian)
};

struct OperatingTimes {
    int workingHours;        // (byte0-1)
    int workingMinutes;      // (byte2)
    int operatingHours;      // (byte3-4)
    int operatingMinutes;    // (byte5)
    int startCounter;        // (byte6-7)
};

struct SubsystemsStatus {
    int glowPlugPower;       // % * 2 (byte0)
    int fuelPumpFrequency;   // Hz * 2 (byte1)
    int combustionFanPower;  // % * 2 (byte2)
    int circulationPumpPower; // % * 2 (byte4)
};

struct ErrorInfo {
    byte errorCode;
    byte count;
    bool isStored;
    bool isActual;
    int operatingState;
    float temperature;
    float voltage;
    int operatingHours;
    int operatingMinutes;
};

// =============================================
// ФУНКЦИИ ДЕКОДИРОВАНИЯ
// =============================================

class WBusDecoders {
public:
    // Декодирование сырого HEX ответа в строку
    static String decodeRawResponse(const String& hexResponse);
    
    // Декодирование статусных флагов (0x50 index 0x02)
    static SensorStatusFlags decodeStatusFlags(const String& hexData);
    
    // Декодирование флагов вкл/выкл (0x50 index 0x03)
    static OnOffFlags decodeOnOffFlags(const String& hexData);
    
    // Декодирование операционных измерений (0x50 index 0x05)
    static OperationalMeasurements decodeOperationalMeasurements(const String& hexData);
    
    // Декодирование времени работы (0x50 index 0x07)
    static OperatingTimes decodeOperatingTimes(const String& hexData);
    
    // Декодирование статуса подсистем (0x50 index 0x0F)
    static SubsystemsStatus decodeSubsystemsStatus(const String& hexData);
    
    // Декодирование состояния устройства (0x50 index 0x06)
    static String decodeOperatingState(byte stateCode);
    
    // Декодирование информации об ошибке (0x56)
    static ErrorInfo decodeErrorInfo(const String& hexData);
    
    // Декодирование WBUS-кода (0x51 index 0x0C)
    static String decodeWbusCode(const String& hexData);
    
    // Декодирование версии W-Bus (0x51 index 0x0A)
    static String decodeWbusVersion(byte versionByte);
    
    // Декодирование температуры (с offset +50°C)
    static float decodeTemperature(byte tempByte);
    
    // Декодирование напряжения (big endian)
    static float decodeVoltage(byte highByte, byte lowByte);
    
    // Вспомогательные функции
    static byte hexStringToByte(const String& hexStr);
    static int hexStringToInt(const String& hexStr);
    static String byteToBinaryString(byte b);

private:
    static bool getBit(byte data, byte bitPosition);
};

#endif // WBUSDECODERS_H