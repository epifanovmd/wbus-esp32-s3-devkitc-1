#include "wbus-info.h"
#include "wbus-queue.h"
#include "wbus-info-decoder.h"
#include "wbus.constants.h"

WebastoInfo webastoInfo;

// Методы обработки ответов
void WebastoInfo::handleWBusVersionResponse(String tx, String rx)
{
    if (!rx.isEmpty())
    {
        DecodedVersion version = wBusDecoder.decodeWBusVersion(rx);
        if (version.isValid)
        {
            deviceInfo.wbusVersion = version.versionString;
            deviceInfo.lastUpdate = millis();
        }
    }
}

void WebastoInfo::handleDeviceNameResponse(String tx, String rx)
{
    if (!rx.isEmpty())
    {
        DecodedTextData name = wBusDecoder.decodeDeviceName(rx);
        if (name.isValid)
        {
            deviceInfo.deviceName = name.text;
            deviceInfo.lastUpdate = millis();
        }
    }
}

void WebastoInfo::handleWBusCodeResponse(String tx, String rx)
{
    if (!rx.isEmpty())
    {
        DecodedWBusCode code = wBusDecoder.decodeWBusCode(rx);
        if (code.isValid)
        {
            deviceInfo.wbusCode = code.codeString;
            deviceInfo.supportedFunctions = code.supportedFunctions;
            deviceInfo.lastUpdate = millis();
        }
    }
}

void WebastoInfo::handleDeviceIDResponse(String tx, String rx)
{
    if (!rx.isEmpty())
    {
        DecodedTextData id = wBusDecoder.decodeDeviceID(rx);
        if (id.isValid)
        {
            deviceInfo.deviceID = id.text;
            deviceInfo.lastUpdate = millis();
        }
    }
}

void WebastoInfo::handleHeaterManufactureDateResponse(String tx, String rx)
{
    if (!rx.isEmpty())
    {
        DecodedManufactureDate date = wBusDecoder.decodeHeaterManufactureDate(rx);
        if (date.isValid)
        {
            deviceInfo.heaterManufactureDate = date.dateString;
            deviceInfo.lastUpdate = millis();
        }
    }
}

void WebastoInfo::handleControllerManufactureDateResponse(String tx, String rx)
{
    if (!rx.isEmpty())
    {
        DecodedManufactureDate date = wBusDecoder.decodeControllerManufactureDate(rx);
        if (date.isValid)
        {
            deviceInfo.controllerManufactureDate = date.dateString;
            deviceInfo.lastUpdate = millis();
        }
    }
}

void WebastoInfo::handleCustomerIDResponse(String tx, String rx)
{
    if (!rx.isEmpty())
    {
        DecodedTextData customerID = wBusDecoder.decodeCustomerID(rx);
        if (customerID.isValid)
        {
            deviceInfo.customerID = customerID.text;
            deviceInfo.lastUpdate = millis();
        }
    }
}

void WebastoInfo::handleSerialNumberResponse(String tx, String rx)
{
    if (!rx.isEmpty())
    {
        DecodedTextData serial = wBusDecoder.decodeSerialNumber(rx);
        if (serial.isValid)
        {
            deviceInfo.serialNumber = serial.text;
            // testStandCode можно добавить если нужно
            deviceInfo.lastUpdate = millis();
        }
    }
}

void WebastoInfo::handleCommandResponse(String tx, String rx)
{
    if (rx.isEmpty())
        return; // Не обрабатываем пустые ответы

    if (tx == CMD_READ_INFO_WBUS_VERSION)
        handleWBusVersionResponse(tx, rx);
    else if (tx == CMD_READ_INFO_DEVICE_NAME)
        handleDeviceNameResponse(tx, rx);
    else if (tx == CMD_READ_INFO_WBUS_CODE)
        handleWBusCodeResponse(tx, rx);
    else if (tx == CMD_READ_INFO_DEVICE_ID)
        handleDeviceIDResponse(tx, rx);
    else if (tx == CMD_READ_INFO_HEATER_MFG_DATE)
        handleHeaterManufactureDateResponse(tx, rx);
    else if (tx == CMD_READ_INFO_CTRL_MFG_DATE)
        handleControllerManufactureDateResponse(tx, rx);
    else if (tx == CMD_READ_INFO_CUSTOMER_ID)
        handleCustomerIDResponse(tx, rx);
    else if (tx == CMD_READ_INFO_SERIAL_NUMBER)
        handleSerialNumberResponse(tx, rx);
    // else
    //     Serial.println("❌ Для этой команды нет обработчика: " + tx);
}

// Публичные методы (остаются без изменений)
void WebastoInfo::getWBusVersion(std::function<void(String, String)> callback)
{
    wbusQueue.add(CMD_READ_INFO_WBUS_VERSION,
                  [this, callback](String tx, String rx)
                  {
                      this->handleCommandResponse(tx, rx);
                      if (callback != nullptr)
                      {
                          callback(tx, rx);
                      }
                  });
}

void WebastoInfo::getDeviceName(std::function<void(String, String)> callback)
{
    wbusQueue.add(CMD_READ_INFO_DEVICE_NAME,
                  [this, callback](String tx, String rx)
                  {
                      this->handleCommandResponse(tx, rx);
                      if (callback != nullptr)
                      {
                          callback(tx, rx);
                      }
                  });
}

void WebastoInfo::getWBusCode(std::function<void(String, String)> callback)
{
    wbusQueue.add(CMD_READ_INFO_WBUS_CODE,
                  [this, callback](String tx, String rx)
                  {
                      this->handleCommandResponse(tx, rx);
                      if (callback != nullptr)
                      {
                          callback(tx, rx);
                      }
                  });
}

void WebastoInfo::getDeviceID(std::function<void(String, String)> callback)
{
    wbusQueue.add(CMD_READ_INFO_DEVICE_ID,
                  [this, callback](String tx, String rx)
                  {
                      this->handleCommandResponse(tx, rx);
                      if (callback != nullptr)
                      {
                          callback(tx, rx);
                      }
                  });
}

void WebastoInfo::getHeaterManufactureDate(std::function<void(String, String)> callback)
{
    wbusQueue.add(CMD_READ_INFO_HEATER_MFG_DATE,
                  [this, callback](String tx, String rx)
                  {
                      this->handleCommandResponse(tx, rx);
                      if (callback != nullptr)
                      {
                          callback(tx, rx);
                      }
                  });
}

void WebastoInfo::getControllerManufactureDate(std::function<void(String, String)> callback)
{
    wbusQueue.add(CMD_READ_INFO_CTRL_MFG_DATE,
                  [this, callback](String tx, String rx)
                  {
                      this->handleCommandResponse(tx, rx);
                      if (callback != nullptr)
                      {
                          callback(tx, rx);
                      }
                  });
}

void WebastoInfo::getCustomerID(std::function<void(String, String)> callback)
{
    wbusQueue.add(CMD_READ_INFO_CUSTOMER_ID,
                  [this, callback](String tx, String rx)
                  {
                      this->handleCommandResponse(tx, rx);
                      if (callback != nullptr)
                      {
                          callback(tx, rx);
                      }
                  });
}

void WebastoInfo::getSerialNumber(std::function<void(String, String)> callback)
{
    wbusQueue.add(CMD_READ_INFO_SERIAL_NUMBER,
                  [this, callback](String tx, String rx)
                  {
                      this->handleCommandResponse(tx, rx);
                      if (callback != nullptr)
                      {
                          callback(tx, rx);
                      }
                  });
}

void WebastoInfo::getMainInfo()
{
    getWBusVersion();
    getDeviceName();
    getWBusCode();
}

void WebastoInfo::getAdditionalInfo()
{
    getDeviceID();
    getControllerManufactureDate();
    getHeaterManufactureDate();
    getCustomerID();
    getSerialNumber();
}

void WebastoInfo::getAllInfo()
{
    getMainInfo();
    getAdditionalInfo();
}

void WebastoInfo::printInfo()
{
    Serial.println();
    Serial.println("═══════════════════════════════════════════════════════════");
    Serial.println("         ИНФОРМАЦИЯ Webasto                                ");
    Serial.println("═══════════════════════════════════════════════════════════");

    Serial.println("Версия W-шины:                              " + deviceInfo.wbusVersion);
    Serial.println("Обозначение устройства:                     " + deviceInfo.deviceName);
    Serial.println("Кодирование W-шины:                         " + deviceInfo.wbusCode);
    Serial.println("Идентификационный код блока управления:     " + deviceInfo.deviceID);
    Serial.println("Дата выпуска отопителя:                     " + deviceInfo.heaterManufactureDate);
    Serial.println("Дата выпуска блока управления:              " + deviceInfo.controllerManufactureDate);
    Serial.println("Идентификационный код клиента:              " + deviceInfo.customerID);
    Serial.println("Серийный номер:                             " + deviceInfo.serialNumber);
    Serial.println("Код испытательного стенда:                  " + deviceInfo.testStandCode);

    Serial.println();
    Serial.println("Поддерживаемые функции:");
    Serial.print(deviceInfo.supportedFunctions);
}

WebastoDeviceInfo WebastoInfo::getDeviceInfo()
{
    return deviceInfo;
}