#include "wbus-info.h"
#include "wbus-queue.h"
#include "wbus.constants.h"

WebastoInfo webastoInfo;

// =============================================================================
// МЕТОДЫ ОБРАБОТКИ ОТВЕТОВ (возвращают указатели на данные)
// =============================================================================

DecodedVersion *WebastoInfo::handleWBusVersionResponse(String rx)
{
    static DecodedVersion version; // static для возврата указателя

    if (!rx.isEmpty())
    {
        version = wBusDecoder.decodeWBusVersion(rx);
        if (version.isValid)
        {
            wbusVersion = version.versionString;
            return &version;
        }
    }
    return nullptr;
}

DecodedTextData *WebastoInfo::handleDeviceNameResponse(String rx)
{
    static DecodedTextData name;

    if (!rx.isEmpty())
    {
        name = wBusDecoder.decodeDeviceName(rx);
        if (name.isValid)
        {
            deviceName = name.text;
            return &name;
        }
    }
    return nullptr;
}

DecodedWBusCode *WebastoInfo::handleWBusCodeResponse(String rx)
{
    static DecodedWBusCode code;

    if (!rx.isEmpty())
    {
        code = wBusDecoder.decodeWBusCode(rx);
        if (code.isValid)
        {
            wbusCode = code.codeString;
            supportedFunctions = code.supportedFunctions;
            return &code;
        }
    }
    return nullptr;
}

DecodedTextData *WebastoInfo::handleDeviceIDResponse(String rx)
{
    static DecodedTextData id;

    if (!rx.isEmpty())
    {
        id = wBusDecoder.decodeDeviceID(rx);
        if (id.isValid)
        {
            deviceID = id.text;
            return &id;
        }
    }
    return nullptr;
}

DecodedManufactureDate *WebastoInfo::handleHeaterManufactureDateResponse(String rx)
{
    static DecodedManufactureDate date;

    if (!rx.isEmpty())
    {
        date = wBusDecoder.decodeHeaterManufactureDate(rx);
        if (date.isValid)
        {
            heaterManufactureDate = date.dateString;
            return &date;
        }
    }
    return nullptr;
}

DecodedManufactureDate *WebastoInfo::handleControllerManufactureDateResponse(String rx)
{
    static DecodedManufactureDate date;

    if (!rx.isEmpty())
    {
        date = wBusDecoder.decodeControllerManufactureDate(rx);
        if (date.isValid)
        {
            controllerManufactureDate = date.dateString;
            return &date;
        }
    }
    return nullptr;
}

DecodedTextData *WebastoInfo::handleCustomerIDResponse(String rx)
{
    static DecodedTextData customer;

    if (!rx.isEmpty())
    {
        customer = wBusDecoder.decodeCustomerID(rx);
        if (customer.isValid)
        {
            customerID = customer.text;
            return &customer;
        }
    }
    return nullptr;
}

DecodedTextData *WebastoInfo::handleSerialNumberResponse(String rx)
{
    static DecodedTextData serial;

    if (!rx.isEmpty())
    {
        serial = wBusDecoder.decodeSerialNumber(rx);
        if (serial.isValid)
        {
            serialNumber = serial.text;
            return &serial;
        }
    }
    return nullptr;
}

// =============================================================================
// МЕТОДЫ ЗАПРОСА ДАННЫХ (с колбэками, передающими данные)
// =============================================================================

void WebastoInfo::getWBusVersion(bool loop, std::function<void(String, String, DecodedVersion *)> callback)
{
    wbusQueue.add(CMD_READ_INFO_WBUS_VERSION, [this, callback](String tx, String rx)
                  {
                      DecodedVersion* data = this->handleWBusVersionResponse(rx);
                      if (callback != nullptr)
                      {
                          callback(tx, rx, data);
                      } }, loop);
}

void WebastoInfo::getDeviceName(bool loop, std::function<void(String, String, DecodedTextData *)> callback)
{
    wbusQueue.add(CMD_READ_INFO_DEVICE_NAME, [this, callback](String tx, String rx)
                  {
                      DecodedTextData* data = this->handleDeviceNameResponse(rx);
                      if (callback != nullptr)
                      {
                          callback(tx, rx, data);
                      } }, loop);
}

void WebastoInfo::getWBusCode(bool loop, std::function<void(String, String, DecodedWBusCode *)> callback)
{
    wbusQueue.add(CMD_READ_INFO_WBUS_CODE, [this, callback](String tx, String rx)
                  {
                      DecodedWBusCode* data = this->handleWBusCodeResponse(rx);
                      if (callback != nullptr)
                      {
                          callback(tx, rx, data);
                      } }, loop);
}

void WebastoInfo::getDeviceID(bool loop, std::function<void(String, String, DecodedTextData *)> callback)
{
    wbusQueue.add(CMD_READ_INFO_DEVICE_ID, [this, callback](String tx, String rx)
                  {
                      DecodedTextData* data = this->handleDeviceIDResponse(rx);
                      if (callback != nullptr)
                      {
                          callback(tx, rx, data);
                      } }, loop);
}

void WebastoInfo::getHeaterManufactureDate(bool loop, std::function<void(String, String, DecodedManufactureDate *)> callback)
{
    wbusQueue.add(CMD_READ_INFO_HEATER_MFG_DATE, [this, callback](String tx, String rx)
                  {
                      DecodedManufactureDate* data = this->handleHeaterManufactureDateResponse(rx);
                      if (callback != nullptr)
                      {
                          callback(tx, rx, data);
                      } }, loop);
}

void WebastoInfo::getControllerManufactureDate(bool loop, std::function<void(String, String, DecodedManufactureDate *)> callback)
{
    wbusQueue.add(CMD_READ_INFO_CTRL_MFG_DATE, [this, callback](String tx, String rx)
                  {
                      DecodedManufactureDate* data = this->handleControllerManufactureDateResponse(rx);
                      if (callback != nullptr)
                      {
                          callback(tx, rx, data);
                      } }, loop);
}

void WebastoInfo::getCustomerID(bool loop, std::function<void(String, String, DecodedTextData *)> callback)
{
    wbusQueue.add(CMD_READ_INFO_CUSTOMER_ID, [this, callback](String tx, String rx)
                  {
                      DecodedTextData* data = this->handleCustomerIDResponse(rx);
                      if (callback != nullptr)
                      {
                          callback(tx, rx, data);
                      } }, loop);
}

void WebastoInfo::getSerialNumber(bool loop, std::function<void(String, String, DecodedTextData *)> callback)
{
    wbusQueue.add(CMD_READ_INFO_SERIAL_NUMBER, [this, callback](String tx, String rx)
                  {
                      DecodedTextData* data = this->handleSerialNumberResponse(rx);
                      if (callback != nullptr)
                      {
                          callback(tx, rx, data);
                      } }, loop);
}

// =============================================================================
// ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ
// =============================================================================

void WebastoInfo::stopInfoRequests()
{
    wbusQueue.removeCommand(CMD_READ_INFO_WBUS_VERSION);
    wbusQueue.removeCommand(CMD_READ_INFO_DEVICE_NAME);
    wbusQueue.removeCommand(CMD_READ_INFO_WBUS_CODE);
    wbusQueue.removeCommand(CMD_READ_INFO_DEVICE_ID);
    wbusQueue.removeCommand(CMD_READ_INFO_HEATER_MFG_DATE);
    wbusQueue.removeCommand(CMD_READ_INFO_CTRL_MFG_DATE);
    wbusQueue.removeCommand(CMD_READ_INFO_CUSTOMER_ID);
    wbusQueue.removeCommand(CMD_READ_INFO_SERIAL_NUMBER);
}

void WebastoInfo::clear()
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
}

void WebastoInfo::printInfo()
{
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
}