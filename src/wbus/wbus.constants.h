#ifndef WBUSCONSTANTS_H
#define WBUSCONSTANTS_H

// =============================================
// БАЗОВЫЕ КОМАНДЫ УПРАВЛЕНИЯ
// =============================================

#define CMD_SHUTDOWN "F4 02 10 E6" // Выключение нагревателя
#define CMD_TURN_ON "F4 03 20 3B ED" // Включение (время в минутах в данных)
#define CMD_PARK_HEAT "F4 03 21 3B ED" // Паркинг-нагрев (время в минутах в данных)  
#define CMD_VENTILATE "F4 03 22 3B EE" // Вентиляция (время в минутах в данных)
#define CMD_SUPP_HEAT "F4 03 23 3B EF" // Дополнительный нагрев (время в минутах в данных)
#define CMD_CIRC_PUMP_CTRL "F4 03 24 3B F0" // Управление циркуляционным насосом
#define CMD_BOOST_MODE "F4 03 25 3B F1" // Boost режим
// =============================================
// КОМАНДЫ ДИАГНОСТИКИ И СЕРВИСА
// =============================================

#define CMD_DIAGNOSTIC "F4 02 38 CE" // Диагностическая команда (инициализация)
#define CMD_KEEP_ALIVE "F4 04 44 21 00 95" // Keep-alive (поддержание режима)
#define CMD_TEST_COMPONENT "F4 02 45 B7" // Тест компонентов
// =============================================
// КОМАНДЫ ЧТЕНИЯ ДАННЫХ - СЕНСОРЫ (0x50)
// =============================================

#define CMD_READ_SENSOR_STATUS_FLAGS "F4 03 50 02 A5" // Чтение датчика - статус флагов
#define CMD_READ_SENSOR_ON_OFF_FLAGS "F4 03 50 03 A4" // Чтение датчика - флаги вкл/выкл подсистем
#define CMD_READ_SENSOR_FUEL_SETTINGS "F4 03 50 04 A3" // Чтение датчика - тип топлива и настройки
#define CMD_READ_SENSOR_OPERATIONAL "F4 03 50 05 A2" // Чтение датчика - операционные измерения (температура, напряжение и т.д.)
#define CMD_READ_SENSOR_OPERATING_STATE "F4 03 50 06 A1" // Чтение датчика - состояние устройства
#define CMD_READ_SENSOR_OPERATING_TIMES "F4 03 50 07 A0" // Чтение датчика - время работы и счетчики
#define CMD_READ_SENSOR_BURN_DURATION "F4 03 50 10 97" // Чтение датчика - продолжительность горения
#define CMD_READ_SENSOR_WORKING_DURATION "F4 03 50 11 96" // Чтение датчика - время работы PH и SH
#define CMD_READ_SENSOR_START_COUNTERS "F4 03 50 12 95" // Чтение датчика - счетчики запусков
#define CMD_READ_SENSOR_TEMP_THRESHOLDS "F4 03 50 13 B4" // Чтение датчика - температурные пороги
#define CMD_READ_SENSOR_SUBSYSTEMS_STATUS "F4 03 50 0F A8" // Чтение датчика - статус подсистем (ТЭНы, насосы, вентиляторы)
#define CMD_READ_SENSOR_FUEL_PREHEAT "F4 03 50 19 AE" // Чтение датчика - сопротивление и мощность подогрева топлива
#define CMD_READ_SENSOR_VENT_DURATION "F4 03 50 18 AF" // Чтение датчика - продолжительность вентиляции
// =============================================
// КОМАНДЫ ЧТЕНИЯ ИНФОРМАЦИИ (0x51)
// =============================================

#define CMD_READ_INFO_DEVICE_ID "F4 03 51 01 A7" // Чтение информации - ID устройства
#define CMD_READ_INFO_HW_VERSION "F4 03 51 02 A6" // Чтение информации - версия железа
#define CMD_READ_INFO_DATASET_ID "F4 03 51 03 A5" // Чтение информации - ID набора данных
#define CMD_READ_INFO_CTRL_MFG_DATE "F4 03 51 04 A4" // Чтение информации - дата производства контроллера
#define CMD_READ_INFO_HEATER_MFG_DATE "F4 03 51 05 A3" // Чтение информации - дата производства нагревателя
#define CMD_READ_INFO_UNKNOWN_06 "F4 03 51 06 A2" // Чтение информации - неизвестно
#define CMD_READ_INFO_CUSTOMER_ID "F4 03 51 07 A1" // Чтение информации - ID клиента (партномер)
#define CMD_READ_INFO_UNKNOWN_09 "F4 03 51 09 9F" // Чтение информации - неизвестно
#define CMD_READ_INFO_WBUS_VERSION "F4 03 51 0A AC" // Чтение информации - версия W-Bus
#define CMD_READ_INFO_DEVICE_NAME "F4 03 51 0B AD" // Чтение информации - имя устройства
#define CMD_READ_INFO_WBUS_CODE "F4 03 51 0C AA" // Чтение информации - WBUS-код (флаги поддерживаемых функций)
#define CMD_READ_INFO_UNKNOWN_0D "F4 03 51 0D A9" // Чтение информации - неизвестно
// =============================================
// КОМАНДЫ ЧТЕНИЯ ДАННЫХ (0x53)
// =============================================

#define CMD_READ_DATA_STATIC "F4 03 53 02 A6" // Чтение данных - статические данные
// =============================================
// КОМАНДЫ ОШИБОК (0x56)
// =============================================

#define CMD_READ_ERRORS_LIST "F4 03 56 01 A0" // Чтение ошибок - список кодов ошибок
#define CMD_READ_ERROR_BLOCK "F4 04 56 02 98 3C" // Чтение ошибок - блок информации об ошибке
#define CMD_CLEAR_ERRORS "F4 03 56 03 A2" // Очистка ошибок
// =============================================
// КОМАНДЫ КАЛИБРОВКИ CO2 (0x57)
// =============================================

#define CMD_CO2_CAL_READ "F4 03 57 01 A1" // Калибровка CO2 - чтение значений
#define CMD_CO2_CAL_WRITE "F4 03 57 03 A3" // Калибровка CO2 - запись значений  
// =============================================
// МАССИВЫ ДЛЯ ГРУППОВОЙ ОБРАБОТКИ
// =============================================

// Команды инициализации Webasto
#define INIT_COMMANDS_COUNT 4
const String INIT_COMMANDS[INIT_COMMANDS_COUNT] = {
    CMD_READ_INFO_WBUS_VERSION,    // Запрос версии W-Bus
    CMD_READ_INFO_DEVICE_NAME,     // Запрос имени устройства
    CMD_READ_INFO_WBUS_CODE,       // Запрос WBUS-кода
    CMD_DIAGNOSTIC                 // Диагностическая команда
};

// Основные команды датчиков для мониторинга
#define SENSOR_COMMANDS_COUNT 8
const String SENSOR_COMMANDS[SENSOR_COMMANDS_COUNT] = {
    CMD_READ_SENSOR_STATUS_FLAGS,      // Статус флагов
    CMD_READ_SENSOR_ON_OFF_FLAGS,      // Флаги вкл/выкл
    CMD_READ_SENSOR_OPERATIONAL,       // Операционные измерения
    CMD_READ_SENSOR_OPERATING_STATE,   // Состояние устройства
    CMD_READ_SENSOR_OPERATING_TIMES,   // Время работы
    CMD_READ_SENSOR_SUBSYSTEMS_STATUS, // Статус подсистем
    CMD_READ_SENSOR_TEMP_THRESHOLDS,   // Температурные пороги
    CMD_READ_SENSOR_START_COUNTERS     // Счетчики запусков
};

// Команды информации об устройстве
#define INFO_COMMANDS_COUNT 5
const String INFO_COMMANDS[INFO_COMMANDS_COUNT] = {
    CMD_READ_INFO_DEVICE_ID,       // ID устройства
    CMD_READ_INFO_HW_VERSION,      // Версия железа
    CMD_READ_INFO_WBUS_VERSION,    // Версия W-Bus
    CMD_READ_INFO_DEVICE_NAME,     // Имя устройства
    CMD_READ_INFO_WBUS_CODE        // WBUS-код
};

#endif // WBUSCONSTANTS_H
