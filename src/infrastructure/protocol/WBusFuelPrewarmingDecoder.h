#pragma once
#include <Arduino.h>
#include "../../domain/Entities.h"
#include "../../common/Utils.h"

class WBusFuelPrewarmingDecoder {
public:
    static FuelPrewarming decode(const String &response) {
        FuelPrewarming result = {0, 0, 0.0f, false};
        
        if (!Utils::validateASCPacketStructure(response, 0x50, 0x13, 7)) {
            return result;
        }

        int byteCount;
        uint8_t *data = Utils::hexStringToByteArray(response, byteCount);

        if (byteCount >= 9) {
            // Байты 4-5: сопротивление (big endian)
            result.resistance = (data[4] << 8) | data[5];
            // Байты 6-7: мощность (big endian)  
            result.power = (data[6] << 8) | data[7];
            
            // Конвертируем в омы
            result.resistanceOhms = result.resistance / 1000.0f;
            // Подогрев активен если мощность > 0
            result.isActive = (result.power > 0);
        }

        return result;
    }
};