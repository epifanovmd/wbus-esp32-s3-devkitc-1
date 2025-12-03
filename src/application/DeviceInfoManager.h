#pragma once
#include "../interfaces/IDeviceInfoManager.h"
#include "../core/EventBus.h"
#include "../infrastructure/protocol/WBusCommandBuilder.h"
#include "../infrastructure/protocol/WBusInfoDecoder.h"
#include "../application/CommandManager.h"

class DeviceInfoManager : public IDeviceInfoManager
{
private:
  EventBus &eventBus;
  CommandManager &commandManager;

  // Данные устройства
  String wbusVersion;
  String deviceName;
  String deviceID;
  String serialNumber;
  String controllerManufactureDate;
  String heaterManufactureDate;
  String customerID;
  String wbusCode;
  String supportedFunctions;

  bool hasData = false;

public:
  DeviceInfoManager(EventBus &bus, CommandManager &cmdManager) : eventBus(bus),
                                                                 commandManager(cmdManager) {}

  void requestAllInfo(bool loop = false) override
  {
    requestWBusVersion(loop);
    requestDeviceName(loop);
    requestWBusCode(loop);
    requestDeviceID(loop);
    requestControllerManufactureDate(loop);
    requestHeaterManufactureDate(loop);
    requestCustomerID(loop);
    requestSerialNumber(loop);
  }

  void requestWBusVersion(bool loop = false, std::function<void(String, String)> callback = nullptr) override
  {
    commandManager.addCommand(WBusCommandBuilder::createReadInfo(WBusCommandBuilder::INFO_WBUS_VERSION), callback, loop);
  }

  void requestDeviceName(bool loop = false, std::function<void(String, String)> callback = nullptr) override
  {
    commandManager.addCommand(WBusCommandBuilder::createReadInfo(WBusCommandBuilder::INFO_DEVICE_NAME), callback, loop);
  }

  void requestWBusCode(bool loop = false, std::function<void(String, String)> callback = nullptr) override
  {
    commandManager.addCommand(WBusCommandBuilder::createReadInfo(WBusCommandBuilder::INFO_WBUS_CODE), callback, loop);
  }

  void requestDeviceID(bool loop = false, std::function<void(String, String)> callback = nullptr) override
  {
    commandManager.addCommand(WBusCommandBuilder::createReadInfo(WBusCommandBuilder::INFO_DEVICE_ID), callback, loop);
  }

  void requestControllerManufactureDate(bool loop = false, std::function<void(String, String)> callback = nullptr) override
  {
    commandManager.addCommand(WBusCommandBuilder::createReadInfo(WBusCommandBuilder::INFO_CTRL_MFG_DATE), callback, loop);
  }

  void requestHeaterManufactureDate(bool loop = false, std::function<void(String, String)> callback = nullptr) override
  {
    commandManager.addCommand(WBusCommandBuilder::createReadInfo(WBusCommandBuilder::INFO_HEATER_MFG_DATE), callback, loop);
  }

  void requestCustomerID(bool loop = false, std::function<void(String, String)> callback = nullptr) override
  {
    commandManager.addCommand(WBusCommandBuilder::createReadInfo(WBusCommandBuilder::INFO_CUSTOMER_ID), callback, loop);
  }

  void requestSerialNumber(bool loop = false, std::function<void(String, String)> callback = nullptr) override
  {
    commandManager.addCommand(WBusCommandBuilder::createReadInfo(WBusCommandBuilder::INFO_SERIAL_NUMBER), callback, loop);
  }

  // =========================================================================

  void handleWBusVersionResponse(String tx, String rx, std::function<void(String, String, String *)> callback = nullptr)
  {
    if (!rx.isEmpty())
    {
      wbusVersion = WBusInfoDecoder::decodeWBusVersion(rx);
      eventBus.publish(EventType::WBUS_VERSION, wbusVersion);

      if (callback)
      {
        callback(tx, rx, &wbusVersion);
      }
    }
  }

  void handleDeviceNameResponse(String tx, String rx, std::function<void(String, String, String *)> callback = nullptr)
  {
    if (!rx.isEmpty())
    {
      deviceName = WBusInfoDecoder::decodeDeviceName(rx);
      eventBus.publish(EventType::DEVICE_NAME, deviceName);

      if (callback)
      {
        callback(tx, rx, &deviceName);
      }
    }
  }

  void handleWBusCodeResponse(String tx, String rx, std::function<void(String, String, DecodedWBusCode *)> callback = nullptr)
  {
    if (!rx.isEmpty())
    {
      DecodedWBusCode code = WBusInfoDecoder::decodeWBusCode(rx);
      wbusCode = code.codeString;
      supportedFunctions = code.supportedFunctions;
      eventBus.publish<DecodedWBusCode>(EventType::WBUS_CODE, code);

      if (callback)
      {
        callback(tx, rx, &code);
      }
    }
  }

  void handleDeviceIDResponse(String tx, String rx, std::function<void(String, String, String *)> callback = nullptr)
  {
    if (!rx.isEmpty())
    {
      deviceID = WBusInfoDecoder::decodeDeviceID(rx);
      eventBus.publish(EventType::DEVICE_ID, deviceID);

      if (callback)
      {
        callback(tx, rx, &deviceID);
      }
    }
  }

  void handleControllerManufactureDateResponse(String tx, String rx, std::function<void(String, String, DecodedManufactureDate *)> callback = nullptr)
  {
    if (!rx.isEmpty())
    {
      DecodedManufactureDate date = WBusInfoDecoder::decodeControllerManufactureDate(rx);
      controllerManufactureDate = date.dateString;
      eventBus.publish<DecodedManufactureDate>(EventType::CONTRALLER_MANUFACTURE_DATE, date);

      if (callback)
      {
        callback(tx, rx, &date);
      }
    }
  }

  void handleHeaterManufactureDateResponse(String tx, String rx, std::function<void(String, String, DecodedManufactureDate *)> callback = nullptr)
  {
    if (!rx.isEmpty())
    {
      DecodedManufactureDate date = WBusInfoDecoder::decodeHeaterManufactureDate(rx);
      heaterManufactureDate = date.dateString;
      eventBus.publish<DecodedManufactureDate>(EventType::HEATER_MANUFACTURE_DATE, date);

      if (callback)
      {
        callback(tx, rx, &date);
      }
    }
  }

  void handleCustomerIDResponse(String tx, String rx, std::function<void(String, String, String *)> callback = nullptr)
  {
    if (!rx.isEmpty())
    {
      customerID = WBusInfoDecoder::decodeCustomerID(rx);
      eventBus.publish(EventType::CUSTOMER_ID, customerID);

      if (callback)
      {
        callback(tx, rx, &customerID);
      }
    }
  }

  void handleSerialNumberResponse(String tx, String rx, std::function<void(String, String, String *)> callback = nullptr)
  {
    if (!rx.isEmpty())
    {

      serialNumber = WBusInfoDecoder::decodeSerialNumber(rx);
      eventBus.publish(EventType::SERIAL_NUMBER, serialNumber);

      if (callback)
      {
        callback(tx, rx, &serialNumber);
      }
    }
  }

  // Геттеры (остаются без изменений)
  String getWBusVersionData() const override { return wbusVersion; }
  String getDeviceNameData() const override { return deviceName; }
  String getDeviceIDData() const override { return deviceID; }
  String getSerialNumberData() const override { return serialNumber; }
  String getControllerManufactureDateData() const override { return controllerManufactureDate; }
  String getHeaterManufactureDateData() const override { return heaterManufactureDate; }
  String getCustomerIDData() const override { return customerID; }
  String getWBusCodeData() const override { return wbusCode; }
  String getSupportedFunctionsData() const override { return supportedFunctions; }

  String getDeviceInfoJson() const override
  {
    String json = "{";
    json += "\"wbusVersion\":\"" + wbusVersion + "\",";
    json += "\"deviceName\":\"" + deviceName + "\",";
    json += "\"deviceId\":\"" + deviceID + "\",";
    json += "\"serialNumber\":\"" + serialNumber + "\",";
    json += "\"controllerManufactureDate\":\"" + controllerManufactureDate + "\",";
    json += "\"heaterManufactureDate\":\"" + heaterManufactureDate + "\",";
    json += "\"customerId\":\"" + customerID + "\",";
    json += "\"wbusCode\":\"" + wbusCode + "\",";
    json += "\"supportedFunctions\":\"" + supportedFunctions + "\"";
    json += "}";
    return json;
  }

  void clear() override
  {
    wbusVersion = "";
    deviceName = "";
    deviceID = "";
    serialNumber = "";
    controllerManufactureDate = "";
    heaterManufactureDate = "";
    customerID = "";
    wbusCode = "";
    supportedFunctions = "";
    hasData = false;
  }
};