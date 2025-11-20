#include "wbus-sensors-decoder.h"
#include "common/utils/utils.h"

WBusSensorsDecoder wBusSensorsDecoder;

// =============================================================================
// ВАЛИДАЦИЯ И ПРЕОБРАЗОВАНИЕ ДАННЫХ
// =============================================================================

bool WBusSensorsDecoder::validatePacketStructure(const String &response, uint8_t expectedCommand, uint8_t expectedIndex, int minLength)
{
    String cleanData = response;
    cleanData.replace(" ", "");

    if (cleanData.length() < minLength * 2)
    {
        return false;
    }

    // Проверяем заголовок и команду
    if (cleanData.substring(0, 2) != "4f")
    {
        return false;
    }

    // Проверяем команду (с установленным битом ACK)
    uint8_t receivedCommand = hexStringToByte(cleanData.substring(4, 6));
    if (receivedCommand != (expectedCommand | 0x80))
    {
        return false;
    }

    // Проверяем индекс
    uint8_t receivedIndex = hexStringToByte(cleanData.substring(6, 8));
    if (receivedIndex != expectedIndex)
    {
        return false;
    }

    return true;
}

// =============================================================================
// ДЕКОДИРОВАНИЕ ОПЕРАЦИОННЫХ ДАННЫХ
// =============================================================================

OperationalMeasurements WBusSensorsDecoder::decodeOperationalInfo(const String &response)
{
    OperationalMeasurements result = {0};

    if (!validatePacketStructure(response, 0x50, 0x05, 13))
    {
        return result;
    }

    int byteCount;
    byte *data = hexStringToByteArray(response, byteCount);

    if (byteCount >= 13)
    {
        result.temperature = data[4] - 50.0;
        result.voltage = (float)((data[5] << 8) | data[6]) / 1000.0;
        result.flameDetected = (data[7] == 0x01);
        result.heatingPower = (data[8] << 8) | data[9];
        result.flameResistance = (data[10] << 8) | data[11];
    }

    return result;
}

// =============================================================================
// ДЕКОДИРОВАНИЕ НАСТРОЕК ТОПЛИВА
// =============================================================================

String WBusSensorsDecoder::determineFuelTypeName(uint8_t fuelType)
{
    switch (fuelType)
    {
    case 0x0D:
        return "Дизельное топливо";
    case 0x1D:
        return "Дизельное топливо (альтернативный код)";
    case 0x2D:
        return "Бензин";
    case 0x03:
        return "Газ";
    case 0x05:
        return "Биотопливо";
    default:
        if (fuelType >= 0x01 && fuelType <= 0x0F)
            return "Дизельные топлива";
        if (fuelType >= 0x10 && fuelType <= 0x2F)
            return "Бензины";
        if (fuelType >= 0x30 && fuelType <= 0x4F)
            return "Газовые топлива";
        return "Неизвестный тип";
    }
}

String WBusSensorsDecoder::determineVentilationDescription(uint8_t ventFactor)
{
    switch (ventFactor)
    {
    case 0x3C:
        return "стандартный (60 мин)";
    case 0x1E:
        return "сокращенный (30 мин)";
    case 0x0A:
        return "минимальный (10 мин)";
    case 0x5A:
        return "увеличенный (90 мин)";
    default:
        return "пользовательский (" + String(ventFactor) + " мин)";
    }
}

FuelSettings WBusSensorsDecoder::decodeFuelSettings(const String &response)
{
    FuelSettings result = {0};

    if (!validatePacketStructure(response, 0x50, 0x04, 7))
    {
        return result;
    }

    int byteCount;
    byte *data = hexStringToByteArray(response, byteCount);

    if (byteCount >= 7)
    {
        result.fuelType = data[4];
        result.maxHeatingTime = data[5];
        result.ventilationFactor = data[6];
        result.fuelTypeName = determineFuelTypeName(result.fuelType);
    }

    return result;
}

// =============================================================================
// ДЕКОДИРОВАНИЕ ФЛАГОВ ВКЛ/ВЫКЛ
// =============================================================================

String WBusSensorsDecoder::buildActiveComponentsString(const OnOffFlags &flags)
{
    String components = "";
    if (flags.combustionAirFan)
        components += "Вентилятор горения, ";
    if (flags.glowPlug)
        components += "Свеча накаливания, ";
    if (flags.fuelPump)
        components += "Топливный насос, ";
    if (flags.circulationPump)
        components += "Циркуляционный насос, ";
    if (flags.vehicleFanRelay)
        components += "Вентилятор автомобиля, ";
    if (flags.nozzleStockHeating)
        components += "Подогрев форсунки, ";
    if (flags.flameIndicator)
        components += "Индикатор пламени, ";

    if (components.length() > 0)
    {
        return components.substring(0, components.length() - 2);
    }
    return "нет активных";
}

OnOffFlags WBusSensorsDecoder::decodeOnOffFlags(const String &response)
{
    OnOffFlags result = {false};

    if (!validatePacketStructure(response, 0x50, 0x03, 5))
    {
        return result;
    }

    int byteCount;
    byte *data = hexStringToByteArray(response, byteCount);

    if (byteCount >= 5)
    {
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

// =============================================================================
// ДЕКОДИРОВАНИЕ СТАТУСНЫХ ФЛАГОВ
// =============================================================================

String WBusSensorsDecoder::buildStatusSummaryString(const StatusFlags &flags)
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

String WBusSensorsDecoder::determineOperationMode(const StatusFlags &flags)
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

StatusFlags WBusSensorsDecoder::decodeStatusFlags(const String &response)
{
    StatusFlags result = {false};

    if (!validatePacketStructure(response, 0x50, 0x02, 8))
    {
        return result;
    }

    int byteCount;
    byte *data = hexStringToByteArray(response, byteCount);

    if (byteCount >= 9)
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

// =============================================================================
// ДЕКОДИРОВАНИЕ СОСТОЯНИЯ РАБОТЫ
// =============================================================================

String WBusSensorsDecoder::getStateName(uint8_t stateCode)
{
    // Та же реализация что и ранее (для экономии места оставлю сокращенную версию)
    switch (stateCode)
    {
    case 0x00:
        return "Продувка";
    case 0x01:
        return "Деактивация";
    case 0x04:
        return "Выключен";
    case 0x05:
        return "Горение частичная нагрузка";
    case 0x06:
        return "Горение полная нагрузка";
    case 0x07:
        return "Подача топлива";
    case 0x1C:
        return "Вентиляция";
    case 0x24:
        return "Старт";
    case 0x41:
        return "Горение паркинг-нагрев";
    case 0x42:
        return "Горение доп. нагрев";
    case 0x51:
        return "Тест компонентов";
    case 0x52:
        return "Boost";
    default:
        return "Неизвестное состояние";
    }
}

String WBusSensorsDecoder::getStateDescription(uint8_t stateCode)
{
    if (stateCode == 0x04)
        return "Нагреватель выключен и готов к работе";
    if (stateCode >= 0x05 && stateCode <= 0x06)
        return "Активный процесс горения";
    if (stateCode >= 0x07 && stateCode <= 0x09)
        return "Фаза подачи топлива";
    if (stateCode >= 0x1C && stateCode <= 0x1D)
        return "Режим вентиляции";
    if (stateCode >= 0x24 && stateCode <= 0x27)
        return "Процесс запуска";
    if (stateCode >= 0x41 && stateCode <= 0x44)
        return "Основной процесс горения";
    return "Промежуточное состояние системы";
}

String WBusSensorsDecoder::decodeDeviceStateFlags(uint8_t flags)
{
    String result = "";
    if (flags & 0x01)
        result += "STFL, ";
    if (flags & 0x02)
        result += "UEHFL, ";
    if (flags & 0x04)
        result += "SAFL, ";
    if (flags & 0x08)
        result += "RZFL, ";

    if (result.length() > 0)
    {
        return result.substring(0, result.length() - 2);
    }
    return "Нет флагов";
}

OperatingState WBusSensorsDecoder::decodeOperatingState(const String &response)
{
    OperatingState result = {0};

    if (!validatePacketStructure(response, 0x50, 0x06, 10))
    {
        return result;
    }

    int byteCount;
    byte *data = hexStringToByteArray(response, byteCount);

    if (byteCount >= 10)
    {
        result.stateCode = data[4];
        result.stateNumber = data[5];
        result.deviceStateFlags = data[6];
        result.stateName = getStateName(result.stateCode);
        result.stateDescription = getStateDescription(result.stateCode);
        result.deviceStateInfo = decodeDeviceStateFlags(result.deviceStateFlags);
    }

    return result;
}

// =============================================================================
// ДЕКОДИРОВАНИЕ СТАТУСА ПОДСИСТЕМ
// =============================================================================

SubsystemsStatus WBusSensorsDecoder::decodeSubsystemsStatus(const String &response)
{
    SubsystemsStatus result = {0};

    if (!validatePacketStructure(response, 0x50, 0x0F, 9))
    {
        return result;
    }

    int byteCount;
    byte *data = hexStringToByteArray(response, byteCount);

    if (byteCount >= 9)
    {
        // Извлекаем данные (байты 4-8)
        result.glowPlugPower = data[4];        // Мощность свечи (% * 2)
        result.fuelPumpFrequency = data[5];    // Частота ТН (Гц * 2)
        result.combustionFanPower = data[6];   // Мощность вентилятора (% * 2)
        result.unknownByte3 = data[7];         // Неизвестный байт
        result.circulationPumpPower = data[8]; // Мощность циркуляционного насоса (% * 2)

        // Вычисляем реальные значения
        result.glowPlugPowerPercent = result.glowPlugPower / 2.0;
        result.fuelPumpFrequencyHz = result.fuelPumpFrequency / 2.0;
        result.combustionFanPowerPercent = result.combustionFanPower / 2.0;
        result.circulationPumpPowerPercent = result.circulationPumpPower / 2.0;
    }

    return result;
}
