// src/infrastructure/protocol/WBusOperatingStateDecoder.h
#pragma once
#include <Arduino.h>
#include "../../domain/Entities.h"
#include "../../common/Utils.h"

class WBusOperatingStateDecoder
{
public:
    static OperatingState decode(const String &response)
    {
        OperatingState result;

        if (!Utils::validateASCPacketStructure(response, 0x50, 0x06, 13))
        {
            return result;
        }

        uint8_t data[MESSAGE_BUFFER_SIZE];
        int byteCount;
        Utils::hexStringToByteArray(response, data, sizeof(data), byteCount);

        // Правильные смещения согласно структуре пакета:
        // [4F][Len][D0][06][StateCode][StateNum][Flags][Unknown...]
        result.stateCode = data[4];        // Код состояния
        result.stateNumber = data[5];      // Номер состояния
        result.deviceStateFlags = data[6]; // Флаги устройства

        // Декодируем полное описание состояния
        decodeStateDetails(result);

        return result;
    }

private:
    static void decodeStateDetails(OperatingState &state)
    {
        // Полное декодирование согласно протоколу
        state.stateName = getStateName(state.stateCode);
        state.stateDescription = getStateDescription(state.stateCode);
        state.deviceStateInfo = decodeDeviceStateFlags(state.deviceStateFlags);
    }

    static String getStateName(uint8_t stateCode)
    {
        switch (stateCode)
        {
        case 0x00:
            return "Продувка";
        case 0x01:
            return "Деактивация";
        case 0x02:
            return "Продувка ADR";
        case 0x03:
            return "Продувка рампы";
        case 0x04:
            return "Выключен";
        case 0x05:
            return "Горение частичная нагрузка";
        case 0x06:
            return "Горение полная нагрузка";
        case 0x07:
            return "Подача топлива";
        case 0x08:
            return "Запуск вентилятора горения";
        case 0x09:
            return "Прерывание подачи топлива";
        case 0x0A:
            return "Диагностика";
        case 0x0B:
            return "Прерывание топливного насоса";
        case 0x0C:
            return "Измерение EMF";
        case 0x0D:
            return "Стабилизация";
        case 0x0E:
            return "Деактивация";
        case 0x0F:
            return "Опрос датчика пламени";

        // Состояния запуска и подготовки
        case 0x10:
            return "Охлаждение датчика пламени";
        case 0x11:
            return "Фаза измерения датчика пламени";
        case 0x12:
            return "Фаза измерения ZUE";
        case 0x13:
            return "Запуск вентилятора";
        case 0x14:
            return "Рампа свечи накаливания";
        case 0x15:
            return "Блокировка нагревателя";
        case 0x16:
            return "Инициализация";
        case 0x17:
            return "Компенсация пузырьков топлива";
        case 0x18:
            return "Холодный запуск вентилятора";
        case 0x19:
            return "Обогащение при холодном пуске";
        case 0x1A:
            return "Охлаждение";
        case 0x1B:
            return "Смена нагрузки PL-FL";
        case 0x1C:
            return "Вентиляция";
        case 0x1D:
            return "Смена нагрузки FL-PL";
        case 0x1E:
            return "Новая инициализация";
        case 0x1F:
            return "Контролируемая работа";

        // Основные рабочие состояния
        case 0x20:
            return "Период контроля";
        case 0x21:
            return "Мягкий старт";
        case 0x22:
            return "Время безопасности";
        case 0x23:
            return "Продувка";
        case 0x24:
            return "Старт";
        case 0x25:
            return "Стабилизация";
        case 0x26:
            return "Стартовая рампа";
        case 0x27:
            return "Отключение питания";
        case 0x28:
            return "Блокировка";
        case 0x29:
            return "Блокировка ADR";
        case 0x2A:
            return "Время стабилизации";
        case 0x2B:
            return "Переход к контролируемой работе";
        case 0x2C:
            return "Состояние решения";
        case 0x2D:
            return "Подача топлива перед пуском";
        case 0x2E:
            return "Накаливание";
        case 0x2F:
            return "Контроль мощности накаливания";
        case 0x30:
            return "Задержка снижения";
        case 0x31:
            return "Медленный запуск вентилятора";
        case 0x32:
            return "Дополнительное накаливание";
        case 0x33:
            return "Прерывание зажигания";
        case 0x34:
            return "Зажигание";
        case 0x35:
            return "Прерывистое накаливание";
        case 0x36:
            return "Мониторинг приложения";
        case 0x37:
            return "Сохранение блокировки в память";
        case 0x38:
            return "Деактивация блокировки нагревателя";
        case 0x39:
            return "Контроль выхода";
        case 0x3A:
            return "Контроль циркуляционного насоса";
        case 0x3B:
            return "Инициализация процессора";
        case 0x3C:
            return "Опрос рассеянного света";
        case 0x3D:
            return "Предстарт";
        case 0x3E:
            return "Предварительное зажигание";
        case 0x3F:
            return "Зажигание пламени";

        // Основные режимы работы
        case 0x40:
            return "Стабилизация пламени";
        case 0x41:
            return "Горение – паркинг-нагрев";
        case 0x42:
            return "Горение – доп. нагрев";
        case 0x43:
            return "Ошибка горения паркинг-нагрев";
        case 0x44:
            return "Ошибка горения доп. нагрев";
        case 0x45:
            return "Выключение после работы";
        case 0x46:
            return "Контроль после работы";
        case 0x47:
            return "Послеродовой режим из-за ошибки";
        case 0x48:
            return "Временной послеродовой режим из-за ошибки";
        case 0x49:
            return "Блокировка циркуляционного насоса";
        case 0x4A:
            return "Контроль простоя после паркинг-нагрева";
        case 0x4B:
            return "Контроль простоя после доп. нагрева";
        case 0x4C:
            return "Контроль простоя с циркуляционным насосом";
        case 0x4D:
            return "Циркуляционный насос без нагрева";
        case 0x4E:
            return "Цикл ожидания перенапряжения";
        case 0x4F:
            return "Обновление памяти ошибок";

        // Дополнительные состояния
        case 0x50:
            return "Цикл ожидания";
        case 0x51:
            return "Тест компонентов";
        case 0x52:
            return "Boost";
        case 0x53:
            return "Охлаждение";
        case 0x54:
            return "Постоянная блокировка нагревателя";
        case 0x55:
            return "Холостой ход вентилятора";
        case 0x56:
            return "Отрыв";
        case 0x57:
            return "Опрос температуры";
        case 0x58:
            return "Предстарт пониженного напряжения";
        case 0x59:
            return "Опрос аварии";
        case 0x5A:
            return "Послеродовой соленоидный клапан";
        case 0x5B:
            return "Обновление ошибок соленоидного клапана";
        case 0x5C:
            return "Временной послеродовой соленоидный клапан";
        case 0x5D:
            return "Попытка запуска";
        case 0x5E:
            return "Расширение предстарта";
        case 0x5F:
            return "Процесс горения";
        case 0x60:
            return "Временной послеродовой режим из-за пониженного напряжения";
        case 0x61:
            return "Обновление ошибок перед выключением";
        case 0x62:
            return "Рампа полной нагрузки";

        default:
            return "Неизвестное состояние (0x" + String(stateCode, HEX) + ")";
        }
    }

    static String getStateDescription(uint8_t stateCode)
    {
        if (stateCode == 0x04)
            return "Нагреватель выключен и готов к работе";
        if (stateCode >= 0x05 && stateCode <= 0x06)
            return "Активный процесс горения";
        if (stateCode >= 0x07 && stateCode <= 0x09)
            return "Фаза подачи топлива";
        if (stateCode >= 0x0A && stateCode <= 0x0C)
            return "Диагностика и измерения";
        if (stateCode >= 0x10 && stateCode <= 0x1F)
            return "Подготовка и запуск системы";
        if (stateCode >= 0x20 && stateCode <= 0x27)
            return "Процесс запуска";
        if (stateCode >= 0x28 && stateCode <= 0x3F)
            return "Контроль и стабилизация";
        if (stateCode >= 0x41 && stateCode <= 0x44)
            return "Основной процесс горения";
        if (stateCode >= 0x45 && stateCode <= 0x4F)
            return "Завершение работы";
        if (stateCode >= 0x51 && stateCode <= 0x52)
            return "Специальные режимы";
        if (stateCode >= 0x54)
            return "Аварийные и блокирующие состояния";

        return "Промежуточное состояние системы";
    }

    static String decodeDeviceStateFlags(uint8_t flags)
    {
        String result = "";
        String details = "";

        // Основные флаги
        if (flags & 0x01)
        {
            result += "STFL (Start Flag), ";
            details += "Система в процессе запуска, ";
        }
        if (flags & 0x02)
        {
            result += "UEHFL (Overheat Flag), ";
            details += "Обнаружен перегрев! ";
        }
        if (flags & 0x04)
        {
            result += "SAFL (Safety Flag), ";
            details += "Система безопасности активна, ";
        }
        if (flags & 0x08)
        {
            result += "RZFL (Run Flag), ";
            details += "Нагреватель в рабочем режиме, ";
        }

        // Дополнительные флаги
        if (flags & 0x10)
            result += "FLAG_0x10, ";
        if (flags & 0x20)
            result += "FLAG_0x20, ";
        if (flags & 0x40)
            result += "FLAG_0x40, ";
        if (flags & 0x80)
            result += "FLAG_0x80, ";

        // Форматирование результата
        if (result.length() > 0)
        {
            result = result.substring(0, result.length() - 2);
        }
        else
        {
            result = "Нет активных флагов";
            details = "Все системы в норме";
        }

        if (details.length() > 0)
        {
            details = details.substring(0, details.length() - 2);
        }

        return result + " | " + details + " [0x" + String(flags, HEX) + "]";
    }
};