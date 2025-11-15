#ifndef WBUS_INFO_H
#define WBUS_INFO_H

#include <Arduino.h>
#include "wbus/wbus-queue.h"
#include "wbus/wbus.constants.h"

// Структура для хранения информации об устройстве
struct WbusDeviceInfo
{
    // Основная информация
    String wbusVersion;
    String deviceName;
    String deviceID;
    String serialNumber;
    String testStandCode;

    // Даты производства
    String controllerManufactureDate;
    String heaterManufactureDate;

    // Коды и идентификаторы
    String customerID;
    String additionalCode;
    String wbusCode;
    String supportedFunctions;

    // Время последнего обновления
    unsigned long lastUpdate;

    // Конструктор
    WbusDeviceInfo()
    {
        clear();
    }

    // Очистка данных
    void clear()
    {
        wbusVersion = "";
        deviceName = "";
        deviceID = "";
        serialNumber = "";
        testStandCode = "";
        controllerManufactureDate = "";
        heaterManufactureDate = "";
        customerID = "";
        additionalCode = "";
        wbusCode = "";
        supportedFunctions = "";

        lastUpdate = 0;
    }

    // Проверка, есть ли данные
    bool hasData()
    {
        return !deviceName.isEmpty();
    }
};

class WebastoInfo
{
public:
    static void getWBusVersion();
    static void getDeviceName();
    static void getWBusCode();
    static void getDeviceID();
    static void getHeaterManufactureDate();
    static void getControllerManufactureDate();
    static void getCustomerID();
    static void getSerialNumber();

    static void getAllInfo();
    static void printInfo();
    static WbusDeviceInfo getDeviceInfo();

private:
    static WbusDeviceInfo deviceInfo;
    static void analyzeWBusCode(const String &codeData);
};

extern WebastoInfo webastoInfo;

#endif // WBUS_INFO_H