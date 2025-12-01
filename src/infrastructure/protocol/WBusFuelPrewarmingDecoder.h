#pragma once
#include <Arduino.h>
#include "../../domain/Entities.h"
#include "../../common/Utils.h"

class WBusFuelPrewarmingDecoder {
public:
    static FuelPrewarming decode(const String &response) {
        FuelPrewarming result = {0, 0, false};
        
        if (!Utils::validateASCPacketStructure(response, 0x50, 0x13, 7)) {
            return result;
        }

        uint8_t data[MESSAGE_BUFFER_SIZE];
        int byteCount;
        Utils::hexStringToByteArray(response, data, sizeof(data), byteCount);

        if (byteCount >= 9) {
            // Байты 4-5: сопротивление (big endian)
            result.resistance = (data[4] << 8) | data[5];
            // Байты 6-7: мощность (big endian)  
            result.power = (data[6] << 8) | data[7];

            // Подогрев активен если мощность > 0
            result.isActive = (result.power > 0);
        }

        return result;
    }
};