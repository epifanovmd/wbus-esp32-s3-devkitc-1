// src/application/DeviceInfoManager.h
#pragma once
#include "../interfaces/IDeviceInfoManager.h"
#include "../core/EventBus.h"
#include "../infrastructure/protocol/WBusInfoDecoder.h"
#include "../application/CommandManager.h"

class DeviceInfoManager: public IDeviceInfoManager {
    private: EventBus & eventBus;
    CommandManager & commandManager;

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

    public: DeviceInfoManager(EventBus & bus, CommandManager & cmdManager): eventBus(bus),
    commandManager(cmdManager) {}

    void requestAllInfo(bool loop = false) override {
      requestWBusVersion(loop);
      requestDeviceName(loop);
      requestWBusCode(loop);
      requestDeviceID(loop);
      requestControllerManufactureDate(loop);
      requestHeaterManufactureDate(loop);
      requestCustomerID(loop);
      requestSerialNumber(loop);
    }

    void requestWBusVersion(bool loop = false, std:: function < void(String, String, DecodedTextData * ) > callback = nullptr) override {
      commandManager.addCommand(WBusProtocol::CMD_READ_INFO_WBUS_VERSION, [this, callback](String tx, String rx) {
        if (!rx.isEmpty()) {
          DecodedTextData version = WBusInfoDecoder::decodeWBusVersion(rx);
          wbusVersion = version.text;
          // Serial.println("✅ W-Bus Version: " + wbusVersion);
          eventBus.publish(EventType::WBUS_VERSION, wbusVersion);

          if (callback) {
            callback(tx, rx, & version);
          }
        }
      }, loop);
    }

    void requestDeviceName(bool loop = false, std:: function < void(String, String, DecodedTextData * ) > callback = nullptr) override {
      commandManager.addCommand(WBusProtocol::CMD_READ_INFO_DEVICE_NAME, [this, callback](String tx, String rx) {
        if (!rx.isEmpty()) {
          DecodedTextData name = WBusInfoDecoder::decodeDeviceName(rx);
          deviceName = name.text;
          // Serial.println("✅ Device Name: " + deviceName);
          eventBus.publish(EventType::DEVICE_NAME, deviceName);

          if (callback) {
            callback(tx, rx, & name);
          }
        }
      }, loop);
    }

    void requestWBusCode(bool loop = false, std:: function < void(String, String, DecodedWBusCode * ) > callback = nullptr) override {
      commandManager.addCommand(WBusProtocol::CMD_READ_INFO_WBUS_CODE, [this, callback](String tx, String rx) {
        if (!rx.isEmpty()) {
          DecodedWBusCode code = WBusInfoDecoder::decodeWBusCode(rx);
          wbusCode = code.codeString;
          supportedFunctions = code.supportedFunctions;
          // Serial.println("✅ W-Bus Code: " + wbusCode);
          eventBus.publish < DecodedWBusCode > (EventType::WBUS_CODE, code);

          if (callback) {
            callback(tx, rx, & code);
          }
        }
      }, loop);
    }

    void requestDeviceID(bool loop = false, std:: function < void(String, String, DecodedTextData * ) > callback = nullptr) override {
      commandManager.addCommand(WBusProtocol::CMD_READ_INFO_DEVICE_ID, [this, callback](String tx, String rx) {
        if (!rx.isEmpty()) {
          DecodedTextData deviceId = WBusInfoDecoder::decodeDeviceID(rx);
          deviceID = deviceId.text;
          // Serial.println("✅ Device ID: " + deviceID);
          eventBus.publish(EventType::DEVICE_ID, deviceID);

          if (callback) {
            callback(tx, rx, & deviceId);
          }
        }
      }, loop);
    }

    void requestControllerManufactureDate(bool loop = false, std:: function < void(String, String, DecodedManufactureDate * ) > callback = nullptr) override {
      commandManager.addCommand(WBusProtocol::CMD_READ_INFO_CTRL_MFG_DATE, [this, callback](String tx, String rx) {
        if (!rx.isEmpty()) {
          DecodedManufactureDate date = WBusInfoDecoder::decodeControllerManufactureDate(rx);
          controllerManufactureDate = date.dateString;
          // Serial.println("✅ Controller Date: " + controllerManufactureDate);
          eventBus.publish < DecodedManufactureDate > (EventType::CONTRALLER_MANUFACTURE_DATE, date);

          if (callback) {
            callback(tx, rx, & date);
          }
        }
      }, loop);
    }

    void requestHeaterManufactureDate(bool loop = false, std:: function < void(String, String, DecodedManufactureDate * ) > callback = nullptr) override {
      commandManager.addCommand(WBusProtocol::CMD_READ_INFO_HEATER_MFG_DATE, [this, callback](String tx, String rx) {
        if (!rx.isEmpty()) {
          DecodedManufactureDate date = WBusInfoDecoder::decodeHeaterManufactureDate(rx);
          heaterManufactureDate = date.dateString;
          // Serial.println("✅ Heater Date: " + heaterManufactureDate);
          eventBus.publish < DecodedManufactureDate > (EventType::HEATER_MANUFACTURE_DATE, date);

          if (callback) {
            callback(tx, rx, & date);
          }
        }
      }, loop);
    }

    void requestCustomerID(bool loop = false, std:: function < void(String, String, DecodedTextData * ) > callback = nullptr) override {
      commandManager.addCommand(WBusProtocol::CMD_READ_INFO_CUSTOMER_ID, [this, callback](String tx, String rx) {
          if (!rx.isEmpty()) {
            DecodedTextData customer = WBusInfoDecoder::decodeCustomerID(rx);
            customerID = customer.text;
            // Serial.println("✅ Customer ID: " + customerID);
            eventBus.publish(EventType::CUSTOMER_ID, customerID);

              if (callback) {
                callback(tx, rx, & customer);
              }
            }
          }, loop);
      }

      void requestSerialNumber(bool loop = false, std:: function < void(String, String, DecodedTextData * ) > callback = nullptr) override {
        commandManager.addCommand(WBusProtocol::CMD_READ_INFO_SERIAL_NUMBER, [this, callback](String tx, String rx) {
          if (!rx.isEmpty()) {
            DecodedTextData serial = WBusInfoDecoder::decodeSerialNumber(rx);
            serialNumber = serial.text;
            // Serial.println("✅ Serial Number: " + serialNumber);
            eventBus.publish(EventType::SERIAL_NUMBER, serialNumber);

            if (callback) {
              callback(tx, rx, & serial);
            }

          }
        }, loop);
      }

      String getWBusVersionData() const override {
        return wbusVersion;
      }
      String getDeviceNameData() const override {
        return deviceName;
      }
      String getDeviceIDData() const override {
        return deviceID;
      }
      String getSerialNumberData() const override {
        return serialNumber;
      }
      String getControllerManufactureDateData() const override {
        return controllerManufactureDate;
      }
      String getHeaterManufactureDateData() const override {
        return heaterManufactureDate;
      }
      String getCustomerIDData() const override {
        return customerID;
      }
      String getWBusCodeData() const override {
        return wbusCode;
      }
      String getSupportedFunctionsData() const override {
        return supportedFunctions;
      }

      String getDeviceInfoJson() const override {
        String json = "{";
        json += "\"wbus_version\":\"" + wbusVersion + "\",";
        json += "\"device_name\":\"" + deviceName + "\",";
        json += "\"device_id\":\"" + deviceID + "\",";
        json += "\"serial_number\":\"" + serialNumber + "\",";
        json += "\"controller_manufacture_date\":\"" + controllerManufactureDate + "\",";
        json += "\"heater_manufacture_date\":\"" + heaterManufactureDate + "\",";
        json += "\"customer_id\":\"" + customerID + "\",";
        json += "\"wbus_code\":\"" + wbusCode + "\",";
        json += "\"supported_functions\":\"" + supportedFunctions + "\"";
        json += "}";
        return json;
      }

      void printInfo() const override {
        Serial.println();
        Serial.println("═══════════════════════════════════════════════════════════");
        Serial.println("         ИНФОРМАЦИЯ Webasto                                ");
        Serial.println("═══════════════════════════════════════════════════════════");

        Serial.println("Версия W-шины:                              " + wbusVersion);
        Serial.println("Обозначение устройства:                     " + deviceName);
        Serial.println("Кодирование W-шины:                         " + wbusCode);
        Serial.println("Идентификационный код блока управления:     " + deviceID);
        Serial.println("Дата выпуска отопителя:                     " + heaterManufactureDate);
        Serial.println("Дата выпуска блока управления:              " + controllerManufactureDate);
        Serial.println("Идентификационный код клиента:              " + customerID);
        Serial.println("Серийный номер:                             " + serialNumber);

        Serial.println();
        Serial.println("Поддерживаемые функции:");
        Serial.print(supportedFunctions);
        Serial.println("═══════════════════════════════════════════════════════════");
        Serial.println();
      }

      void clear() override {
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

      private:
    };