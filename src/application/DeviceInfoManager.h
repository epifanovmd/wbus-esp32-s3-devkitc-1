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
  String wbusVersion = "N/A";
  String deviceName = "N/A";
  String deviceID = "N/A";
  String serialNumber = "N/A";
  String controllerManufactureDate = "N/A";
  String heaterManufactureDate = "N/A";
  String customerID = "N/A";
  DecodedWBusCode wBusCode;

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

  void requestWBusVersion(bool loop = false) override
  {
    commandManager.addCommand(WBusCommandBuilder::createReadInfo(WBusCommandBuilder::INFO_WBUS_VERSION), loop);
  }

  void requestDeviceName(bool loop = false) override
  {
    commandManager.addCommand(WBusCommandBuilder::createReadInfo(WBusCommandBuilder::INFO_DEVICE_NAME), loop);
  }

  void requestWBusCode(bool loop = false) override
  {
    commandManager.addCommand(WBusCommandBuilder::createReadInfo(WBusCommandBuilder::INFO_WBUS_CODE), loop);
  }

  void requestDeviceID(bool loop = false) override
  {
    commandManager.addCommand(WBusCommandBuilder::createReadInfo(WBusCommandBuilder::INFO_DEVICE_ID), loop);
  }

  void requestControllerManufactureDate(bool loop = false) override
  {
    commandManager.addCommand(WBusCommandBuilder::createReadInfo(WBusCommandBuilder::INFO_CTRL_MFG_DATE), loop);
  }

  void requestHeaterManufactureDate(bool loop = false) override
  {
    commandManager.addCommand(WBusCommandBuilder::createReadInfo(WBusCommandBuilder::INFO_HEATER_MFG_DATE), loop);
  }

  void requestCustomerID(bool loop = false) override
  {
    commandManager.addCommand(WBusCommandBuilder::createReadInfo(WBusCommandBuilder::INFO_CUSTOMER_ID), loop);
  }

  void requestSerialNumber(bool loop = false) override
  {
    commandManager.addCommand(WBusCommandBuilder::createReadInfo(WBusCommandBuilder::INFO_SERIAL_NUMBER), loop);
  }

  // =========================================================================

  void handleWBusVersionResponse(String tx, String rx, std::function<void(String, String, String *)> callback = nullptr)
  {
    wbusVersion = WBusInfoDecoder::decodeWBusVersion(rx);
    eventBus.publish(EventType::WBUS_VERSION, wbusVersion);
  }

  void handleDeviceNameResponse(String tx, String rx)
  {
    deviceName = WBusInfoDecoder::decodeDeviceName(rx);
    eventBus.publish(EventType::DEVICE_NAME, deviceName);
  }

  void handleWBusCodeResponse(String tx, String rx)
  {
    wBusCode = WBusInfoDecoder::decodeWBusCode(rx);
    eventBus.publish<DecodedWBusCode>(EventType::WBUS_CODE, wBusCode);
  }

  void handleDeviceIDResponse(String tx, String rx)
  {
    deviceID = WBusInfoDecoder::decodeDeviceID(rx);
    eventBus.publish(EventType::DEVICE_ID, deviceID);
  }

  void handleControllerManufactureDateResponse(String tx, String rx)
  {
    DecodedManufactureDate date = WBusInfoDecoder::decodeControllerManufactureDate(rx);
    controllerManufactureDate = date.dateString;
    eventBus.publish<DecodedManufactureDate>(EventType::CONTRALLER_MANUFACTURE_DATE, date);
  }

  void handleHeaterManufactureDateResponse(String tx, String rx)
  {
    DecodedManufactureDate date = WBusInfoDecoder::decodeHeaterManufactureDate(rx);
    heaterManufactureDate = date.dateString;
    eventBus.publish<DecodedManufactureDate>(EventType::HEATER_MANUFACTURE_DATE, date);
  }

  void handleCustomerIDResponse(String tx, String rx)
  {
    customerID = WBusInfoDecoder::decodeCustomerID(rx);
    eventBus.publish(EventType::CUSTOMER_ID, customerID);
  }

  void handleSerialNumberResponse(String tx, String rx)
  {
    serialNumber = WBusInfoDecoder::decodeSerialNumber(rx);
    eventBus.publish(EventType::SERIAL_NUMBER, serialNumber);
  }

  // Геттеры (остаются без изменений)
  String getWBusVersionData() const override { return wbusVersion; }
  String getDeviceNameData() const override { return deviceName; }
  String getDeviceIDData() const override { return deviceID; }
  String getSerialNumberData() const override { return serialNumber; }
  String getControllerManufactureDateData() const override { return controllerManufactureDate; }
  String getHeaterManufactureDateData() const override { return heaterManufactureDate; }
  String getCustomerIDData() const override { return customerID; }
  DecodedWBusCode getWBusCodeData() const override { return wBusCode; }

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
    json += "\"wBusCode\":" + wBusCode.toJson();
    json += "}";
    return json;
  }

  void clear() override
  {
    wbusVersion = "N/A";
    deviceName = "N/A";
    deviceID = "N/A";
    serialNumber = "N/A";
    controllerManufactureDate = "N/A";
    heaterManufactureDate = "N/A";
    customerID = "N/A";
    wBusCode.clear();
    hasData = false;
  }
};