#ifndef WBUSCONSTANTS_H
#define WBUSCONSTANTS_H

#include "common/utils/utils.h"

// =================================================================================
// Структура пакета:
// [ Заголовок | Длина | Команда | Данные ... | Контрольная сумма ]
// =================================================================================

// =================================================================================
// БАЗОВЫЕ КОМАНДЫ УПРАВЛЕНИЯ (время в минутах указывается в данных – 3B = 59 минут)
// =================================================================================

#define CMD_SHUTDOWN "F4 02 10 E6"          // Выключение нагревателя
#define CMD_PARK_HEAT "F4 03 21 3B ED"      // Включение на 59 минут
#define CMD_VENTILATE "F4 03 22 3B EE"      // Вентиляция 59 минут
#define CMD_SUPP_HEAT "F4 03 23 3B EF"      // Дополнительный нагрев
#define CMD_CIRC_PUMP_CTRL "F4 03 24 3B F0" // Управление циркуляционным насосом
#define CMD_BOOST_MODE "F4 03 25 3B F1"     // Boost режим

// Функция для создания команды паркинг-нагрева
inline String createParkHeatCommand(uint8_t minutes = 59)
{
    minutes = constrain(minutes, 1, 255);
    byte data[] = {0xF4, 0x03, 0x21, minutes};
    byte checksum = calculateChecksum(data, 4);
    return "F4 03 21 " + byteToHexString(minutes) + " " + byteToHexString(checksum);
}

// Функция для создания команды вентиляции
inline String createVentilateCommand(uint8_t minutes = 59)
{
    minutes = constrain(minutes, 1, 255);
    byte data[] = {0xF4, 0x03, 0x22, minutes};
    byte checksum = calculateChecksum(data, 4);
    return "F4 03 22 " + byteToHexString(minutes) + " " + byteToHexString(checksum);
}

// Функция для создания команды дополнительного нагрева
inline String createSuppHeatCommand(uint8_t minutes = 59)
{
    minutes = constrain(minutes, 1, 255);
    byte data[] = {0xF4, 0x03, 0x23, minutes};
    byte checksum = calculateChecksum(data, 4);
    return "F4 03 23 " + byteToHexString(minutes) + " " + byteToHexString(checksum);
}

// Функция для создания команды управления циркуляционным насосом
inline String createCircPumpCommand(bool enable)
{
    uint8_t dataByte = enable ? 0x01 : 0x00;
    byte data[] = {0xF4, 0x03, 0x24, dataByte};
    byte checksum = calculateChecksum(data, 4);
    return "F4 03 24 " + byteToHexString(dataByte) + " " + byteToHexString(checksum);
}

// Функция для создания команды boost режима
inline String createBoostCommand(uint8_t minutes = 59)
{
    minutes = constrain(minutes, 1, 255);
    byte data[] = {0xF4, 0x03, 0x25, minutes};
    byte checksum = calculateChecksum(data, 4);
    return "F4 03 25 " + byteToHexString(minutes) + " " + byteToHexString(checksum);
}

// =================================================================================
// КОМАНДЫ ДИАГНОСТИКИ И СЕРВИСА
// =================================================================================

#define CMD_DIAGNOSTIC "F4 02 38 CE"               // Диагностическая команда (инициализация)
#define CMD_KEEPALIVE_PARKING "F4 04 44 21 00 95"   // Keep-alive для паркинга
#define CMD_KEEPALIVE_VENT "F4 04 44 22 00 96"      // Keep-alive для вентиляции
#define CMD_KEEPALIVE_SUPP_HEAT "F4 04 44 23 00 97" // Keep-alive для дополнительного нагрева
#define CMD_KEEPALIVE_CIRC_PUMP "F4 04 44 24 00 90" // Keep-alive для циркуляционного насоса
#define CMD_KEEPALIVE_BOOST "F4 04 44 25 00 91"     // Keep-alive для Boost режима

// =================================================================================
// МАКРОСЫ ДЛЯ ТЕСТИРОВАНИЯ КОМПОНЕНТОВ С ДИНАМИЧЕСКИМ РАСЧЕТОМ CRC
// =================================================================================

// Функция для создания команды тестирования компонента
inline String createTestCommand(uint8_t component, uint8_t seconds, uint16_t magnitude)
{
    // Формируем массив байт для расчета контрольной суммы
    // Команда состоит из 8 байт: header + length + command + component + seconds + magnitude_high + magnitude_low + CRC
    byte data[] = {
        0xF4, 0x06, 0x45, component,
        seconds,
        (byte)(magnitude >> 8),
        (byte)(magnitude & 0xFF)};
    byte checksum = calculateChecksum(data, 7);

    return "F4 06 45 " +
           byteToHexString(component) + " " +
           byteToHexString(seconds) + " " +
           byteToHexString(magnitude >> 8) + " " +
           byteToHexString(magnitude & 0xFF) + " " +
           byteToHexString(checksum);
}

// Функции для конкретных компонентов
inline String createTestCAFCommand(uint8_t seconds, uint8_t powerPercent)
{
    uint16_t magnitude = powerPercent * 2; // 1% = 2 единицы (50% = 0x64)
    return createTestCommand(0x01, seconds, magnitude);
}

inline String createTestFuelPumpCommand(uint8_t seconds, uint8_t frequencyHz)
{
    uint16_t magnitude = frequencyHz * 20; // 1 Гц = 20 единиц (10 Гц = 0xC8)
    return createTestCommand(0x02, seconds, magnitude);
}

inline String createTestGlowPlugCommand(uint8_t seconds, uint8_t powerPercent)
{
    uint16_t magnitude = powerPercent * 2; // 1% = 2 единицы (75% = 0x96)
    return createTestCommand(0x03, seconds, magnitude);
}

inline String createTestCircPumpCommand(uint8_t seconds, uint8_t powerPercent)
{
    uint16_t magnitude = powerPercent * 2; // 1% = 2 единицы (100% = 0xC8)
    return createTestCommand(0x04, seconds, magnitude);
}

inline String createTestVehicleFanCommand(uint8_t seconds)
{
    return createTestCommand(0x05, seconds, 0x0001); // всегда 0x0001 для реле
}

inline String createTestSolenoidCommand(uint8_t seconds)
{
    return createTestCommand(0x09, seconds, 0x0001); // всегда 0x0001 для соленоида
}

inline String createTestFuelPreheatCommand(uint8_t seconds, uint8_t powerPercent)
{
    uint16_t magnitude = powerPercent * 2; // 1% = 2 единицы (50% = 0x64)
    return createTestCommand(0x0F, seconds, magnitude);
}

// =================================================================================
// КОМАНДЫ ЧТЕНИЯ ДАННЫХ - СЕНСОРЫ (0x50)
// =================================================================================

#define CMD_READ_SENSOR_STATUS_FLAGS "F4 03 50 02 A5"    // Чтение датчика - статус флагов
#define CMD_READ_SENSOR_ON_OFF_FLAGS "F4 03 50 03 A4"    // Чтение датчика - флаги вкл/выкл подсистем
#define CMD_READ_SENSOR_FUEL_SETTINGS "F4 03 50 04 A3"   // Чтение датчика - тип топлива и настройки
#define CMD_READ_SENSOR_OPERATIONAL "F4 03 50 05 A2"     // Чтение датчика - операционные измерения (температура, напряжение и т.д.)
#define CMD_READ_SENSOR_OPERATING_STATE "F4 03 50 06 A1" // Чтение датчика - состояние устройства
// #define CMD_READ_SENSOR_OPERATING_TIMES "F4 03 50 07 A0" // Чтение датчика - время работы и счетчики
// #define CMD_READ_SENSOR_BURN_DURATION "F4 03 50 10 97" // Чтение датчика - продолжительность горения
// #define CMD_READ_SENSOR_WORKING_DURATION "F4 03 50 11 96" // Чтение датчика - время работы PH и SH
// #define CMD_READ_SENSOR_START_COUNTERS "F4 03 50 12 95" // Чтение датчика - счетчики запусков
// #define CMD_READ_SENSOR_TEMP_THRESHOLDS "F4 03 50 13 B4" // Чтение датчика - температурные пороги
#define CMD_READ_SENSOR_SUBSYSTEMS_STATUS "F4 03 50 0F A8" // Чтение датчика - статус подсистем (ТЭНы, насосы, вентиляторы)
// #define CMD_READ_SENSOR_FUEL_PREHEAT "F4 03 50 19 AE" // Чтение датчика - сопротивление и мощность подогрева топлива
// #define CMD_READ_SENSOR_VENT_DURATION "F4 03 50 18 AF" // Чтение датчика - продолжительность вентиляции

// =================================================================================
// КОМАНДЫ ЧТЕНИЯ ИНФОРМАЦИИ (0x51)
// =================================================================================

#define CMD_READ_INFO_DEVICE_ID "F4 03 51 01 A7" // Чтение информации - ID устройства
// #define CMD_READ_INFO_HW_VERSION "F4 03 51 02 A6" // Чтение информации - версия железа
// #define CMD_READ_INFO_DATASET_ID "F4 03 51 03 A5" // Чтение информации - ID набора данных
#define CMD_READ_INFO_CTRL_MFG_DATE "F4 03 51 04 A2"   // Чтение информации - дата производства контроллера
#define CMD_READ_INFO_HEATER_MFG_DATE "F4 03 51 05 A3" // Чтение информации - дата производства нагревателя
#define CMD_READ_INFO_CUSTOMER_ID "F4 03 51 07 A1"     // Чтение информации - ID клиента (партномер)
#define CMD_READ_INFO_SERIAL_NUMBER "F4 03 51 09 AF"   // Чтение серийного номера и кода исполнительного стенда
#define CMD_READ_INFO_WBUS_VERSION "F4 03 51 0A AC"    // Чтение информации - версия W-Bus
#define CMD_READ_INFO_DEVICE_NAME "F4 03 51 0B AD"     // Чтение информации - имя устройства
#define CMD_READ_INFO_WBUS_CODE "F4 03 51 0C AA"       // Чтение информации - WBUS-код (флаги поддерживаемых функций)

// =================================================================================
// КОМАНДЫ ОШИБОК (0x56)
// =================================================================================

#define CMD_READ_ERRORS_LIST "F4 03 56 01 A0" // Чтение ошибок - список кодов ошибок
// #define CMD_READ_ERROR_BLOCK "F4 04 56 02 98 3C" // Чтение ошибок - блок информации об ошибке
#define CMD_CLEAR_ERRORS "F4 03 56 03 A2" // Очистка ошибок

// =================================================================================
// КОМАНДЫ КАЛИБРОВКИ CO2 (0x57)
// =================================================================================

// #define CMD_CO2_CAL_READ "F4 03 57 01 A1" // Калибровка CO2 - чтение значений
// #define CMD_CO2_CAL_WRITE "F4 03 57 03 A3" // Калибровка CO2 - запись значений

#endif // WBUSCONSTANTS_H