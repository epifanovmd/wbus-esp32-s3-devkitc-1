#pragma once

class IDeviceInfoManager
{
public:
    virtual ~IDeviceInfoManager() = default;

    virtual void requestAllInfo(bool loop = false) = 0;
    virtual void requestWBusVersion(bool loop = false, std::function<void(String, String)> callback = nullptr) = 0;
    virtual void requestDeviceName(bool loop = false, std::function<void(String, String)> callback = nullptr) = 0;
    virtual void requestWBusCode(bool loop = false, std::function<void(String, String)> callback = nullptr) = 0;
    virtual void requestDeviceID(bool loop = false, std::function<void(String, String)> callback = nullptr) = 0;
    virtual void requestControllerManufactureDate(bool loop = false, std::function<void(String, String)> callback = nullptr) = 0;
    virtual void requestHeaterManufactureDate(bool loop = false, std::function<void(String, String)> callback = nullptr) = 0;
    virtual void requestCustomerID(bool loop = false, std::function<void(String, String)> callback = nullptr) = 0;
    virtual void requestSerialNumber(bool loop = false, std::function<void(String, String)> callback = nullptr) = 0;

    virtual String getWBusVersionData() const = 0;
    virtual String getDeviceNameData() const = 0;
    virtual String getDeviceIDData() const = 0;
    virtual String getSerialNumberData() const = 0;
    virtual String getControllerManufactureDateData() const = 0;
    virtual String getHeaterManufactureDateData() const = 0;
    virtual String getCustomerIDData() const = 0;
    virtual String getWBusCodeData() const = 0;
    virtual String getSupportedFunctionsData() const = 0;

    virtual String getDeviceInfoJson() const = 0;
    virtual void clear() = 0;
};