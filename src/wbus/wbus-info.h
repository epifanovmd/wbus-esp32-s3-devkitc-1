#ifndef WBUS_INFO_H
#define WBUS_INFO_H

#include <Arduino.h>
#include "wbus/wbus-queue.h"
#include "wbus/wbus.constants.h"

// Структура для хранения информации об устройстве
struct WebastoDeviceInfo
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
    WebastoDeviceInfo()
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
private:
    WebastoDeviceInfo deviceInfo;
    String analyzeWBusCodeByte(uint8_t byteNum, uint8_t byteVal);
    void analyzeWBusCode(const String &codeData);

    // Приватные методы для callback'ов
    void handleWBusVersionResponse(bool status, String tx, String rx);
    void handleDeviceNameResponse(bool status, String tx, String rx);
    void handleWBusCodeResponse(bool status, String tx, String rx);
    void handleDeviceIDResponse(bool status, String tx, String rx);
    void handleHeaterManufactureDateResponse(bool status, String tx, String rx);
    void handleControllerManufactureDateResponse(bool status, String tx, String rx);
    void handleCustomerIDResponse(bool status, String tx, String rx);
    void handleSerialNumberResponse(bool status, String tx, String rx);

public:
    void getWBusVersion();
    void getDeviceName();
    void getWBusCode();
    void getDeviceID();
    void getHeaterManufactureDate();
    void getControllerManufactureDate();
    void getCustomerID();
    void getSerialNumber();

    void getAllInfo();
    void printInfo();
    WebastoDeviceInfo getDeviceInfo();
};

extern WebastoInfo webastoInfo;

#endif // WBUS_INFO_H