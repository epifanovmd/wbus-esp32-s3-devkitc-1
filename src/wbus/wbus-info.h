#ifndef WBUS_INFO_H
#define WBUS_INFO_H

#include <Arduino.h>

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

    // Приватные методы для callback'ов
    void handleWBusVersionResponse(String tx, String rx);
    void handleDeviceNameResponse(String tx, String rx);
    void handleWBusCodeResponse(String tx, String rx);
    void handleDeviceIDResponse(String tx, String rx);
    void handleHeaterManufactureDateResponse(String tx, String rx);
    void handleControllerManufactureDateResponse(String tx, String rx);
    void handleCustomerIDResponse(String tx, String rx);
    void handleSerialNumberResponse(String tx, String rx);

public:
    void getWBusVersion(std::function<void(String, String)> callback = nullptr);
    void getDeviceName(std::function<void(String, String)> callback = nullptr);
    void getWBusCode(std::function<void(String, String)> callback = nullptr);
    void getDeviceID(std::function<void(String, String)> callback = nullptr);
    void getHeaterManufactureDate(std::function<void(String, String)> callback = nullptr);
    void getControllerManufactureDate(std::function<void(String, String)> callback = nullptr);
    void getCustomerID(std::function<void(String, String)> callback = nullptr);
    void getSerialNumber(std::function<void(String, String)> callback = nullptr);

    // универсальная функция обработки, по tx выбирает нужный обработчик
    bool handleCommandResponse(String tx, String rx);

    // аггрегирующие функции
    void getMainInfo();
    void getAdditionalInfo();
    void getAllInfo();
    void printInfo();

    void clear();

    WebastoDeviceInfo getDeviceInfo();
    bool hasDeviceInfo() { return deviceInfo.hasData(); }
};

extern WebastoInfo webastoInfo;

#endif // WBUS_INFO_H