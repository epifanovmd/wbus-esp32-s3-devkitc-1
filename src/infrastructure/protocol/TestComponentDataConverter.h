#pragma once
#include <Arduino.h>
#include "../../common/Utils.h"

// =========================================================================
// КЛАСС ДЛЯ ПРЕОБРАЗОВАНИЯ ПАРАМЕТРОВ ТЕСТИРОВАНИЯ
// =========================================================================
class TestComponentConverter
{
public:
    struct TestCommandInfo
    {
        uint8_t component;
        uint8_t seconds;
        uint16_t magnitude;

        TestCommandInfo() : component(0), seconds(0), magnitude(0) {}
    };

    static TestCommandInfo decodeTestCommand(const String &txCommand)
    {
        TestCommandInfo info;

        // Формат команды: F4 06 45 [COMPONENT] [SECONDS] [MAGNITUDE_MSB] [MAGNITUDE_LSB] [CHECKSUM]

        // Проверяем минимальную длину (8 байт = 16 символов)
        if (txCommand.length() < 16)
        { // Учитываем пробелы в исходной строке
            return info;
        }

        // Извлекаем компонент (4-й байт в команде, индекс 3)
        info.component = Utils::extractByteFromString(txCommand, 3);
        // Извлекаем время в секундах (5-й байт, индекс 4)
        info.seconds = Utils::extractByteFromString(txCommand, 4);

        // Извлекаем величину (2 байта, индексы 5 и 6)
        uint8_t magnitudeMsb = Utils::extractByteFromString(txCommand, 5);
        uint8_t magnitudeLsb = Utils::extractByteFromString(txCommand, 6);
        info.magnitude = (magnitudeMsb << 8) | magnitudeLsb;

        return info;
    }

    // =========================================================================
    // КОНВЕРТЕРЫ ДЛЯ КАЖДОГО КОМПОНЕНТА (процент → величина)
    // =========================================================================

    // Вентилятор горения: 0-100% → 0-510 (0.5% на единицу)
    static uint16_t combustionFanPercentToMagnitude(uint8_t percent)
    {
        percent = constrain(percent, 0, 100);
        float value_f = (percent * 510.0f) / 100.0f;
        return static_cast<uint16_t>(value_f + 0.5f);
    }

    // Обратное преобразование: величина → процент
    static uint8_t combustionFanMagnitudeToPercent(uint16_t magnitude)
    {
        float percent_f = (magnitude * 100.0f) / 510.0f;
        return static_cast<uint8_t>(percent_f + 0.5f);
    }

    // =========================================================================

    // Топливный насос: Гц → величина (1 Гц = 2 единицы)
    static uint16_t fuelPumpHzToMagnitude(uint8_t frequencyHz)
    {
        frequencyHz = constrain(frequencyHz, 0, 255);
        return frequencyHz;
    }

    // Обратное преобразование: величина → Гц
    static uint8_t fuelPumpMagnitudeToHz(uint16_t magnitude)
    {
        return magnitude / 2;
    }

    // =========================================================================

    // Свеча накаливания: 0-100% → 0-200 (0.5% на единицу)
    static uint16_t glowPlugPercentToMagnitude(uint8_t percent)
    {
        percent = constrain(percent, 0, 100);
        return static_cast<uint16_t>((percent * 200UL) / 100UL);
    }

    // Обратное преобразование: величина → процент
    static uint8_t glowPlugMagnitudeToPercent(uint16_t magnitude)
    {
        return static_cast<uint8_t>((magnitude * 100UL) / 200UL);
    }

    // =========================================================================

    // Циркуляционный насос: 0-100% → 0-200 (0.5% на единицу)
    static uint16_t circulationPumpPercentToMagnitude(uint8_t percent)
    {
        percent = constrain(percent, 0, 100);
        return percent * 2;
    }

    // Обратное преобразование: величина → процент
    static uint8_t circulationPumpMagnitudeToPercent(uint16_t magnitude)
    {
        return magnitude / 2;
    }

    // =========================================================================

    // Подогрев топлива: 0-100% → 0-510 (0.5% на единицу)
    static uint16_t fuelPreheatingPercentToMagnitude(uint8_t percent)
    {
        percent = constrain(percent, 0, 100);
        float value_f = (percent * 510.0f) / 100.0f;
        return static_cast<uint16_t>(value_f + 0.5f);
    }

    // Обратное преобразование: величина → процент
    static uint8_t fuelPreheatingMagnitudeToPercent(uint16_t magnitude)
    {
        float percent_f = (magnitude * 100.0f) / 510.0f;
        return static_cast<uint8_t>(percent_f + 0.5f);
    }

    // =========================================================================
    // УНИВЕРСАЛЬНЫЕ ФУНКЦИИ ДЛЯ КОМАНД БЕЗ ПАРАМЕТРОВ
    // =========================================================================

    // Вентилятор автомобиля: всегда 0x0001
    static uint16_t vehicleFanToMagnitude()
    {
        return 0x0001;
    }

    // =========================================================================

    // Соленоидный клапан: всегда 0x0001
    static uint16_t solenoidValveToMagnitude()
    {
        return 0x0001;
    }
};