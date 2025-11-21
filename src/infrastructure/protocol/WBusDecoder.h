// src/infrastructure/protocol/WBusDecoder.h
#pragma once
#include <Arduino.h>
#include "../../domain/Entities.h"
#include "../../common/Utils.h"

// Структуры из оригинального wbus-sensors-decoder.h
struct FuelSettings {
    uint8_t fuelType = 0;
    uint8_t maxHeatingTime = 0;
    uint8_t ventilationFactor = 0;
    String fuelTypeName = "";
};

struct OnOffFlags {
    bool combustionAirFan = false;
    bool glowPlug = false;
    bool fuelPump = false;
    bool circulationPump = false;
    bool vehicleFanRelay = false;
    bool nozzleStockHeating = false;
    bool flameIndicator = false;
    String activeComponents = "";
};

struct StatusFlags {
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
    String statusSummary = "";
    String operationMode = "";
};

struct OperatingState {
    uint8_t stateCode = 0;
    uint8_t stateNumber = 0;
    uint8_t deviceStateFlags = 0;
    String stateName = "";
    String stateDescription = "";
    String deviceStateInfo = "";
};

struct SubsystemsStatus {
    uint8_t glowPlugPower = 0;
    uint8_t fuelPumpFrequency = 0;
    uint8_t combustionFanPower = 0;
    uint8_t unknownByte3 = 0;
    uint8_t circulationPumpPower = 0;
    
    float glowPlugPowerPercent = 0;
    float fuelPumpFrequencyHz = 0;
    float combustionFanPowerPercent = 0;
    float circulationPumpPowerPercent = 0;
};

class WBusDecoder {
public:
    static OperationalMeasurements decodeOperationalInfo(const String& response) {
        OperationalMeasurements result;
        
        if (!validatePacketStructure(response, 0x50, 0x05, 13)) {
            return result;
        }
        
        int byteCount;
        byte* data = Utils::hexStringToByteArray(response, byteCount);
        
        if (byteCount >= 13) {
            result.temperature = data[4] - 50.0;
            result.voltage = (float)((data[5] << 8) | data[6]) / 1000.0;
            result.flameDetected = (data[7] == 0x01);
            result.heatingPower = (data[8] << 8) | data[9];
            result.flameResistance = (data[10] << 8) | data[11];
        }
        
        return result;
    }
    
    static FuelSettings decodeFuelSettings(const String& response) {
        FuelSettings result;
        
        if (!validatePacketStructure(response, 0x50, 0x04, 7)) {
            return result;
        }
        
        int byteCount;
        byte* data = Utils::hexStringToByteArray(response, byteCount);
        
        if (byteCount >= 7) {
            result.fuelType = data[4];
            result.maxHeatingTime = data[5];
            result.ventilationFactor = data[6];
            result.fuelTypeName = determineFuelTypeName(result.fuelType);
        }
        
        return result;
    }
    
    static OnOffFlags decodeOnOffFlags(const String& response) {
        OnOffFlags result;
        
        if (!validatePacketStructure(response, 0x50, 0x03, 5)) {
            return result;
        }
        
        int byteCount;
        byte* data = Utils::hexStringToByteArray(response, byteCount);
        
        if (byteCount >= 5) {
            byte flags = data[4];
            result.combustionAirFan = (flags & 0x01) != 0;
            result.glowPlug = (flags & 0x02) != 0;
            result.fuelPump = (flags & 0x04) != 0;
            result.circulationPump = (flags & 0x08) != 0;
            result.vehicleFanRelay = (flags & 0x10) != 0;
            result.nozzleStockHeating = (flags & 0x20) != 0;
            result.flameIndicator = (flags & 0x40) != 0;
            result.activeComponents = buildActiveComponentsString(result);
        }
        
        return result;
    }
    
    static StatusFlags decodeStatusFlags(const String& response) {
        StatusFlags result;
        
        if (!validatePacketStructure(response, 0x50, 0x02, 8)) {
            return result;
        }
        
        int byteCount;
        byte* data = Utils::hexStringToByteArray(response, byteCount);
        
        if (byteCount >= 9) {
            // Байт 0
            result.mainSwitch = (data[4] & 0x01) != 0;
            result.supplementalHeatRequest = (data[4] & 0x10) != 0;
            result.parkingHeatRequest = (data[4] & 0x20) != 0;
            result.ventilationRequest = (data[4] & 0x40) != 0;
            
            // Байт 1
            result.summerMode = (data[5] & 0x01) != 0;
            result.externalControl = (data[5] & 0x02) != 0;
            
            // Байт 2
            result.generatorSignal = (data[6] & 0x10) != 0;
            
            // Байт 3
            result.boostMode = (data[7] & 0x10) != 0;
            result.auxiliaryDrive = (data[7] & 0x01) != 0;
            
            // Байт 4
            result.ignitionSignal = (data[8] & 0x01) != 0;
            
            result.statusSummary = buildStatusSummaryString(result);
            result.operationMode = determineOperationMode(result);
        }
        
        return result;
    }
    
private:
    static bool validatePacketStructure(const String& response, uint8_t expectedCommand, uint8_t expectedIndex, int minLength) {
        String cleanData = response;
        cleanData.replace(" ", "");
        
        if (cleanData.length() < minLength * 2) {
            return false;
        }
        
        // Проверяем заголовок и команду
        if (cleanData.substring(0, 2) != "4f") {
            return false;
        }
        
        // Проверяем команду (с установленным битом ACK)
        uint8_t receivedCommand = Utils::hexStringToByte(cleanData.substring(4, 6));
        if (receivedCommand != (expectedCommand | 0x80)) {
            return false;
        }
        
        // Проверяем индекс
        uint8_t receivedIndex = Utils::hexStringToByte(cleanData.substring(6, 8));
        if (receivedIndex != expectedIndex) {
            return false;
        }
        
        return true;
    }
    
    static String determineFuelTypeName(uint8_t fuelType) {
        switch (fuelType) {
            case 0x0D: return "Дизельное топливо";
            case 0x1D: return "Дизельное топливо (альтернативный код)";
            case 0x2D: return "Бензин";
            case 0x03: return "Газ";
            case 0x05: return "Биотопливо";
            default:
                if (fuelType >= 0x01 && fuelType <= 0x0F) return "Дизельные топлива";
                if (fuelType >= 0x10 && fuelType <= 0x2F) return "Бензины";
                if (fuelType >= 0x30 && fuelType <= 0x4F) return "Газовые топлива";
                return "Неизвестный тип";
        }
    }
    
    static String buildActiveComponentsString(const OnOffFlags& flags) {
        String components = "";
        if (flags.combustionAirFan) components += "Вентилятор горения, ";
        if (flags.glowPlug) components += "Свеча накаливания, ";
        if (flags.fuelPump) components += "Топливный насос, ";
        if (flags.circulationPump) components += "Циркуляционный насос, ";
        if (flags.vehicleFanRelay) components += "Вентилятор автомобиля, ";
        if (flags.nozzleStockHeating) components += "Подогрев форсунки, ";
        if (flags.flameIndicator) components += "Индикатор пламени, ";
        
        if (components.length() > 0) {
            return components.substring(0, components.length() - 2);
        }
        return "нет активных";
    }
    
    static String buildStatusSummaryString(const StatusFlags& flags) {
        String summary = "";
        if (flags.mainSwitch) summary += "Включен, ";
        if (flags.ignitionSignal) summary += "Зажигание, ";
        if (flags.generatorSignal) summary += "Генератор, ";
        if (flags.summerMode) summary += "Лето, ";
        if (flags.externalControl) summary += "Внешнее управление, ";
        
        if (summary.length() > 0) {
            return summary.substring(0, summary.length() - 2);
        }
        return "базовый статус";
    }
    
    static String determineOperationMode(const StatusFlags& flags) {
        if (flags.parkingHeatRequest) return "🚗 Паркинг-нагрев";
        if (flags.supplementalHeatRequest) return "🔥 Дополнительный нагрев";
        if (flags.ventilationRequest) return "💨 Вентиляция";
        if (flags.boostMode) return "⚡ Boost режим";
        return "💤 Ожидание";
    }
};