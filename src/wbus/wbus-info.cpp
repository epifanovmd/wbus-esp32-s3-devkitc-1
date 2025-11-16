#include "wbus-info.h"

WebastoInfo webastoInfo;

// Методы обработки ответов
void WebastoInfo::handleWBusVersionResponse(bool status, String tx, String rx)
{
    if (!status)
        return;

    DecodedVersion version = wBusDecoder.decodeWBusVersion(rx);
    if (version.isValid)
    {
        deviceInfo.wbusVersion = version.versionString;
        deviceInfo.lastUpdate = millis();
    }
}

void WebastoInfo::handleDeviceNameResponse(bool status, String tx, String rx)
{
    if (!status)
        return;

    DecodedTextData name = wBusDecoder.decodeDeviceName(rx);
    if (name.isValid)
    {
        deviceInfo.deviceName = name.text;
        deviceInfo.lastUpdate = millis();
    }
}

void WebastoInfo::handleWBusCodeResponse(bool status, String tx, String rx)
{
    if (!status)
        return;

    DecodedWBusCode code = wBusDecoder.decodeWBusCode(rx);
    if (code.isValid)
    {
        deviceInfo.wbusCode = code.codeString;
        deviceInfo.supportedFunctions = code.supportedFunctions;
        deviceInfo.lastUpdate = millis();
    }
}

void WebastoInfo::handleDeviceIDResponse(bool status, String tx, String rx)
{
    if (!status)
        return;

    DecodedTextData id = wBusDecoder.decodeDeviceID(rx);
    if (id.isValid)
    {
        deviceInfo.deviceID = id.text;
        deviceInfo.lastUpdate = millis();
    }
}

void WebastoInfo::handleHeaterManufactureDateResponse(bool status, String tx, String rx)
{
    if (!status)
        return;

    DecodedManufactureDate date = wBusDecoder.decodeHeaterManufactureDate(rx);
    if (date.isValid)
    {
        deviceInfo.heaterManufactureDate = date.dateString;
        deviceInfo.lastUpdate = millis();
    }
}

void WebastoInfo::handleControllerManufactureDateResponse(bool status, String tx, String rx)
{
    if (!status)
        return;

    DecodedManufactureDate date = wBusDecoder.decodeControllerManufactureDate(rx);
    if (date.isValid)
    {
        deviceInfo.controllerManufactureDate = date.dateString;
        deviceInfo.lastUpdate = millis();
    }
}

void WebastoInfo::handleCustomerIDResponse(bool status, String tx, String rx)
{
    if (!status)
        return;

    DecodedTextData customerID = wBusDecoder.decodeCustomerID(rx);
    if (customerID.isValid)
    {
        deviceInfo.customerID = customerID.text;
        deviceInfo.lastUpdate = millis();
    }
}

void WebastoInfo::handleSerialNumberResponse(bool status, String tx, String rx)
{
    if (!status)
        return;

    DecodedTextData serial = wBusDecoder.decodeSerialNumber(rx);
    if (serial.isValid)
    {
        deviceInfo.serialNumber = serial.text;
        // testStandCode можно добавить если нужно
        deviceInfo.lastUpdate = millis();
    }
}

// Публичные методы (остаются без изменений)
void WebastoInfo::getWBusVersion()
{
    wbusQueue.add(CMD_READ_INFO_WBUS_VERSION,
                  [this](bool status, String tx, String rx)
                  {
                      this->handleWBusVersionResponse(status, tx, rx);
                  });
}

void WebastoInfo::getDeviceName()
{
    wbusQueue.add(CMD_READ_INFO_DEVICE_NAME,
                  [this](bool status, String tx, String rx)
                  {
                      this->handleDeviceNameResponse(status, tx, rx);
                  });
}

void WebastoInfo::getWBusCode()
{
    wbusQueue.add(CMD_READ_INFO_WBUS_CODE,
                  [this](bool status, String tx, String rx)
                  {
                      this->handleWBusCodeResponse(status, tx, rx);
                  });
}

void WebastoInfo::getDeviceID()
{
    wbusQueue.add(CMD_READ_INFO_DEVICE_ID,
                  [this](bool status, String tx, String rx)
                  {
                      this->handleDeviceIDResponse(status, tx, rx);
                  });
}

void WebastoInfo::getHeaterManufactureDate()
{
    wbusQueue.add(CMD_READ_INFO_HEATER_MFG_DATE,
                  [this](bool status, String tx, String rx)
                  {
                      this->handleHeaterManufactureDateResponse(status, tx, rx);
                  });
}

void WebastoInfo::getControllerManufactureDate()
{
    wbusQueue.add(CMD_READ_INFO_CTRL_MFG_DATE,
                  [this](bool status, String tx, String rx)
                  {
                      this->handleControllerManufactureDateResponse(status, tx, rx);
                  });
}

void WebastoInfo::getCustomerID()
{
    wbusQueue.add(CMD_READ_INFO_CUSTOMER_ID,
                  [this](bool status, String tx, String rx)
                  {
                      this->handleCustomerIDResponse(status, tx, rx);
                  });
}

void WebastoInfo::getSerialNumber()
{
    wbusQueue.add(CMD_READ_INFO_SERIAL_NUMBER,
                  [this](bool status, String tx, String rx)
                  {
                      this->handleSerialNumberResponse(status, tx, rx);
                  });
}

void WebastoInfo::getMainInfo()
{
    getWBusVersion();
    getDeviceName();
    getWBusCode();
}

void WebastoInfo::getAllInfo()
{
    getMainInfo();

    getDeviceID();
    getControllerManufactureDate();
    getHeaterManufactureDate();
    getCustomerID();
    getSerialNumber();
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