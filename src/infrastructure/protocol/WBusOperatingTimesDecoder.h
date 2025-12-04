#pragma once
#include <Arduino.h>
#include "../../domain/Entities.h"
#include "../../common/Utils.h"

class WBusOperatingTimesDecoder
{
public:
    static OperatingTimes decode(const String &response)
    {
        OperatingTimes result = {0, 0, 0, 0, 0};

        Serial.println("OperatingTimes response = " + response);

        if (!Utils::validateASCPacketStructure(response, 0x50, WBusCommandBuilder::SENSOR_OPERATING_TIMES, 12))
        {
            return result;
        }

        uint8_t data[MESSAGE_BUFFER_SIZE];
        int byteCount;
        Utils::hexStringToByteArray(response, data, sizeof(data), byteCount);

        Serial.printf("[OperatingTimesDecoder] Raw data [4-11]: ");
        for (int i = 0; i < byteCount; i++) {
            Serial.printf("%02X ", data[i]);
        }
        Serial.println();

        if (byteCount >= 12)
        {
        // 1. Working hours (2 байта, big-endian)
        result.workingHours = (data[4] << 8) | data[5];
        
        // 2. Working minutes (1 байт)
        result.workingMinutes = data[6];
        
        // 3. Operating hours (2 байта, big-endian)
        result.operatingHours = (data[7] << 8) | data[8];
        
        // 4. Operating minutes (1 байт)
        result.operatingMinutes = data[9];
        
        // 5. Start counter (2 байта, big-endian) - добавляем в структуру
        result.startCounter = ((data[10] << 8) | data[11]);

        Serial.println("data[10] - " + String(data[10], HEX));
        Serial.println("data[11] - " + String(data[11], HEX));
        Serial.println("data[10] << 8 - " + String(data[10] << 8, HEX));
        Serial.println("(data[10] << 8) | data[11] - " + String((data[10] << 8) | data[11], HEX));
        Serial.println("json" + result.toJson());
    

        // Отладочный вывод
        Serial.printf("[OperatingTimesDecoder] Decoded: WH=%d, WM=%d, OH=%d, OM=%d, SC=%d\n",
                     result.workingHours, result.workingMinutes,
                     result.operatingHours, result.operatingMinutes,
                     result.startCounter);
        }

        return result;
    }
};