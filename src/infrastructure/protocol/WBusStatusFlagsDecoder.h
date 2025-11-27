// src/infrastructure/protocol/WBusStatusFlagsDecoder.h
#pragma once
#include <Arduino.h>
#include "../../domain/Entities.h"
#include "../../common/Utils.h"

class WBusStatusFlagsDecoder
{
public:
    static StatusFlags decode(const String &response)
    {
        StatusFlags result;

        if (!Utils::validateASCPacketStructure(response, 0x50, 0x02, 10))
        {
            return result;
        }

        int byteCount;
        uint8_t *data = Utils::hexStringToByteArray(response, byteCount);

        if (byteCount >= 10)
        {
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
    static String buildStatusSummaryString(const StatusFlags &flags)
    {
        String summary = "";
        if (flags.mainSwitch)
            summary += "Включен, ";
        if (flags.ignitionSignal)
            summary += "Зажигание, ";
        if (flags.generatorSignal)
            summary += "Генератор, ";
        if (flags.summerMode)
            summary += "Лето, ";
        if (flags.externalControl)
            summary += "Внешнее управление, ";

        if (summary.length() > 0)
        {
            return summary.substring(0, summary.length() - 2);
        }
        return "базовый статус";
    }

    static String determineOperationMode(const StatusFlags &flags)
    {
        if (flags.parkingHeatRequest)
            return "🚗 Паркинг-нагрев";
        if (flags.supplementalHeatRequest)
            return "🔥 Дополнительный нагрев";
        if (flags.ventilationRequest)
            return "💨 Вентиляция";
        if (flags.boostMode)
            return "⚡ Boost режим";
        return "💤 Ожидание";
    }
};