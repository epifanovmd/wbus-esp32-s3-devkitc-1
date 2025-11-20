#ifndef WBUS_INFO_H
#define WBUS_INFO_H

#include <Arduino.h>
#include "wbus-info-decoder.h"

class WebastoInfo
{
private:
    String wbusVersion;
    String deviceName;
    String deviceID;
    String serialNumber;
    String controllerManufactureDate;
    String heaterManufactureDate;
    String customerID;
    String wbusCode;
    String supportedFunctions;

public:
    // Методы обработки ответов возвращают структуры данных
    DecodedVersion* handleWBusVersionResponse(String rx);
    DecodedTextData* handleDeviceNameResponse(String rx);
    DecodedWBusCode* handleWBusCodeResponse(String rx);
    DecodedTextData* handleDeviceIDResponse(String rx);
    DecodedManufactureDate* handleHeaterManufactureDateResponse(String rx);
    DecodedManufactureDate* handleControllerManufactureDateResponse(String rx);
    DecodedTextData* handleCustomerIDResponse(String rx);
    DecodedTextData* handleSerialNumberResponse(String rx);

    // Универсальный метод обработки команды
    bool handleCommandResponse(String tx, String rx);

    // Методы запроса данных с колбэками
    void getWBusVersion(bool loop = false, std::function<void(String, String, DecodedVersion*)> callback = nullptr);
    void getDeviceName(bool loop = false, std::function<void(String, String, DecodedTextData*)> callback = nullptr);
    void getWBusCode(bool loop = false, std::function<void(String, String, DecodedWBusCode*)> callback = nullptr);
    void getDeviceID(bool loop = false, std::function<void(String, String, DecodedTextData*)> callback = nullptr);
    void getHeaterManufactureDate(bool loop = false, std::function<void(String, String, DecodedManufactureDate*)> callback = nullptr);
    void getControllerManufactureDate(bool loop = false, std::function<void(String, String, DecodedManufactureDate*)> callback = nullptr);
    void getCustomerID(bool loop = false, std::function<void(String, String, DecodedTextData*)> callback = nullptr);
    void getSerialNumber(bool loop = false, std::function<void(String, String, DecodedTextData*)> callback = nullptr);

    void printInfo();
    void clear();
    void stopInfoRequests();

    // Геттеры для внутренних данных
    bool hasData() { return !deviceName.isEmpty(); }
    
    String getWBusVersionData() { return wbusVersion; }
    String getDeviceNameData() { return deviceName; }
    String getDeviceIDData() { return deviceID; }
    String getSerialNumberData() { return serialNumber; }
    String getControllerManufactureDateData() { return controllerManufactureDate; }
    String getHeaterManufactureDateData() { return heaterManufactureDate; }
    String getCustomerIDData() { return customerID; }
    String getWBusCodeData() { return wbusCode; }
    String getSupportedFunctionsData() { return supportedFunctions; }
};

extern WebastoInfo webastoInfo;

#endif // WBUS_INFO_H