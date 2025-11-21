#pragma once
#include "../../common/Utils.h"

// Команды из оригинального wbus.constants.h
namespace WBusConstants {
    // Базовые команды управления
    const String CMD_SHUTDOWN = "F4 02 10 E6";
    const String CMD_DIAGNOSTIC = "F4 02 38 CE";
    
    // Команды чтения сенсоров
    const String CMD_READ_SENSOR_OPERATIONAL = "F4 03 50 05 A2";
    const String CMD_READ_SENSOR_FUEL_SETTINGS = "F4 03 50 04 A3";
    const String CMD_READ_SENSOR_ON_OFF_FLAGS = "F4 03 50 03 A4";
    const String CMD_READ_SENSOR_STATUS_FLAGS = "F4 03 50 02 A5";
    const String CMD_READ_SENSOR_OPERATING_STATE = "F4 03 50 06 A1";
    const String CMD_READ_SENSOR_SUBSYSTEMS_STATUS = "F4 03 50 0F A8";
    
    // Команды информации устройства
    const String CMD_READ_INFO_WBUS_VERSION = "F4 03 51 0A AC";
    const String CMD_READ_INFO_DEVICE_NAME = "F4 03 51 0B AD";
    const String CMD_READ_INFO_WBUS_CODE = "F4 03 51 0C AA";
    const String CMD_READ_INFO_DEVICE_ID = "F4 03 51 01 A7";
    const String CMD_READ_INFO_CTRL_MFG_DATE = "F4 03 51 04 A2";
    const String CMD_READ_INFO_HEATER_MFG_DATE = "F4 03 51 05 A3";
    const String CMD_READ_INFO_CUSTOMER_ID = "F4 03 51 07 A1";
    const String CMD_READ_INFO_SERIAL_NUMBER = "F4 03 51 09 AF";
    
    // Команды ошибок
    const String CMD_READ_ERRORS_LIST = "F4 03 56 01 A0";
    const String CMD_CLEAR_ERRORS = "F4 03 56 03 A2";
    
    // Keep-alive команды
    const String CMD_KEEPALIVE_PARKING = "F4 04 44 21 00 95";
    const String CMD_KEEPALIVE_VENT = "F4 04 44 22 00 96";
    const String CMD_KEEPALIVE_SUPP_HEAT = "F4 04 44 23 00 97";
    const String CMD_KEEPALIVE_CIRC_PUMP = "F4 04 44 24 00 90";
    const String CMD_KEEPALIVE_BOOST = "F4 04 44 25 00 91";
}