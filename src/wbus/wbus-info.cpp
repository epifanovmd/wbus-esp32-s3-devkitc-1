#include "wbus-info.h"

WebastoInfo webastoInfo;

String WebastoInfo::analyzeWBusCodeByte(uint8_t byteNum, uint8_t byteVal)
{
    String result = "";

    switch (byteNum)
    {
    case 0: // Byte 0
        // if (byteVal & 0x01)
        //     result += "  • Unknown (ZH)\n";
        if (byteVal & 0x08)
            result += "  • Простое вкл/выкл\n";
        if (byteVal & 0x10)
            result += "  • Паркинг-нагрев\n";
        if (byteVal & 0x20)
            result += "  • Дополнительный нагрев\n";
        if (byteVal & 0x40)
            result += "  • Вентиляция\n";
        if (byteVal & 0x80)
            result += "  • Boost режим\n";
        break;

    case 1: // Byte 1
        if (byteVal & 0x02)
            result += "  • Внешнее управление цирк. насосом\n";
        if (byteVal & 0x04)
            result += "  • Вентилятор горения (CAV)\n";
        if (byteVal & 0x08)
            result += "  • Свеча накаливания\n";
        if (byteVal & 0x10)
            result += "  • Топливный насос (FP)\n";
        if (byteVal & 0x20)
            result += "  • Циркуляционный насос (CP)\n";
        if (byteVal & 0x40)
            result += "  • Реле вентилятора автомобиля (VFR)\n";
        if (byteVal & 0x80)
            result += "  • Желтый LED\n";
        break;

    case 2: // Byte 2
        if (byteVal & 0x01)
            result += "  • Зеленый LED\n";
        if (byteVal & 0x02)
            result += "  • Искровой разрядник (нет свечи)\n";
        if (byteVal & 0x04)
            result += "  • Соленоидный клапан\n";
        if (byteVal & 0x08)
            result += "  • Индикатор вспомогательного привода\n";
        if (byteVal & 0x10)
            result += "  • Сигнал генератора D+\n";
        if (byteVal & 0x20)
            result += "  • Вентилятор в RPM (не в %)\n";
        break;

    case 3: // Byte 3
        if (byteVal & 0x02)
            result += "  • Калибровка CO2\n";
        if (byteVal & 0x08)
            result += "  • Индикатор работы (OI)\n";
        break;

    case 4: // Byte 4
        // if (byteVal & 0x0F)
        //     result += "  • Unknown (ZH)\n";
        if (byteVal & 0x10)
            result += "  • Мощность в ваттах (иначе в %)\n";
        // if (byteVal & 0x20)
        //     result += "  • Unknown (ZH)\n";
        if (byteVal & 0x40)
            result += "  • Индикатор пламени (FI)\n";
        if (byteVal & 0x80)
            result += "  • Подогрев форсунки\n";
        break;

    case 5: // Byte 5
        if (byteVal & 0x20)
            result += "  • Флаг зажигания (T15)\n";
        if (byteVal & 0x40)
            result += "  • Доступны температурные пороги\n";
        if (byteVal & 0x80)
            result += "  • Чтение подогрева топлива\n";
        break;

    case 6: // Byte 6
        if (byteVal & 0x02)
            result += "  • Уставки: сопр. пламени, об. вентилятора, темп. выхода\n";
        break;
    }

    return result;
}

void WebastoInfo::analyzeWBusCode(const String &codeData)
{
    deviceInfo.wbusCode = codeData;

    String functions = "";
    for (int byteNum = 0; byteNum < 7; byteNum++)
    {
        if (byteNum * 2 + 2 <= codeData.length())
        {
            String byteStr = codeData.substring(byteNum * 2, byteNum * 2 + 2);
            uint8_t byteVal = strtoul(byteStr.c_str(), NULL, 16);
            functions += analyzeWBusCodeByte(byteNum, byteVal);
        }
    }

    deviceInfo.supportedFunctions = functions;
}

// Методы обработки ответов
void WebastoInfo::handleWBusVersionResponse(bool status, String tx, String rx)
{
    if (!status)
        return;

    rx.replace(" ", "");
    rx.toLowerCase();

    int start = rx.indexOf("d10a") + 4;
    if (start >= 4 && rx.length() >= start + 2)
    {
        String versionData = rx.substring(start, start + 2);
        uint8_t versionByte = strtoul(versionData.c_str(), NULL, 16);
        uint8_t major = (versionByte >> 4) & 0x0F;
        uint8_t minor = versionByte & 0x0F;

        deviceInfo.wbusVersion = String(major) + "." + String(minor);
        deviceInfo.lastUpdate = millis();
    }
}

void WebastoInfo::handleDeviceNameResponse(bool status, String tx, String rx)
{
    if (!status)
        return;

    rx.replace(" ", "");
    rx.toLowerCase();

    int start = rx.indexOf("d10b") + 4;
    if (start >= 4)
    {
        String nameData = rx.substring(start, rx.length() - 2);

        String deviceName = "";
        for (int i = 0; i < nameData.length(); i += 2)
        {
            if (i + 2 <= nameData.length())
            {
                String byteStr = nameData.substring(i, i + 2);
                char c = (char)strtoul(byteStr.c_str(), NULL, 16);
                if (c >= 32 && c <= 126)
                {
                    deviceName += c;
                }
            }
        }

        deviceInfo.deviceName = deviceName;
        deviceInfo.lastUpdate = millis();
    }
}

void WebastoInfo::handleWBusCodeResponse(bool status, String tx, String rx)
{
    if (!status)
        return;

    rx.replace(" ", "");
    rx.toLowerCase();

    int start = rx.indexOf("d10c") + 4;
    if (start >= 4)
    {
        String codeData = rx.substring(start, rx.length() - 2);
        analyzeWBusCode(codeData);
        deviceInfo.lastUpdate = millis();
    }
}

void WebastoInfo::handleDeviceIDResponse(bool status, String tx, String rx)
{
    if (!status)
        return;

    rx.replace(" ", "");
    rx.toLowerCase();

    int start = rx.indexOf("d101") + 4;
    if (start >= 4)
    {
        String idData = rx.substring(start, rx.length() - 2);
        deviceInfo.deviceID = idData;
        deviceInfo.lastUpdate = millis();
    }
}

void WebastoInfo::handleHeaterManufactureDateResponse(bool status, String tx, String rx)
{
    if (!status)
        return;

    rx.replace(" ", "");
    rx.toLowerCase();

    int start = rx.indexOf("d105") + 4;
    if (start >= 4 && rx.length() >= start + 6)
    {
        String dateData = rx.substring(start, start + 6);
        String dayStr = dateData.substring(0, 2);
        String monthStr = dateData.substring(2, 4);
        String yearStr = dateData.substring(4, 6);

        deviceInfo.heaterManufactureDate = dayStr + "." + monthStr + ".20" + yearStr;
        deviceInfo.lastUpdate = millis();
    }
}

void WebastoInfo::handleControllerManufactureDateResponse(bool status, String tx, String rx)
{
    if (!status)
        return;

    rx.replace(" ", "");
    rx.toLowerCase();

    int start = rx.indexOf("d104") + 4;
    if (start >= 4 && rx.length() >= start + 6)
    {
        String dateData = rx.substring(start, start + 6);
        String dayStr = dateData.substring(0, 2);
        String monthStr = dateData.substring(2, 4);
        String yearStr = dateData.substring(4, 6);

        deviceInfo.controllerManufactureDate = dayStr + "." + monthStr + ".20" + yearStr;
        deviceInfo.lastUpdate = millis();
    }
}

void WebastoInfo::handleCustomerIDResponse(bool status, String tx, String rx)
{
    if (!status)
        return;

    rx.replace(" ", "");
    rx.toLowerCase();

    int start = rx.indexOf("d107") + 4;
    if (start >= 4)
    {
        String data = rx.substring(start, rx.length() - 2);

        String customerID = "";
        for (int i = 0; i < data.length(); i += 2)
        {
            if (i + 2 <= data.length())
            {
                String byteStr = data.substring(i, i + 2);
                if (byteStr == "00")
                    break;
                char c = (char)strtoul(byteStr.c_str(), NULL, 16);
                if (c >= 32 && c <= 126)
                {
                    customerID += c;
                }
            }
        }

        deviceInfo.customerID = customerID;
        deviceInfo.lastUpdate = millis();
    }
}

void WebastoInfo::handleSerialNumberResponse(bool status, String tx, String rx)
{
    if (!status)
        return;

    rx.replace(" ", "");
    rx.toLowerCase();

    int start = rx.indexOf("d109") + 4;
    if (start >= 4)
    {
        String data = rx.substring(start, rx.length() - 2);
        deviceInfo.serialNumber = data.substring(0, 10);
        deviceInfo.testStandCode = data.substring(10);
        deviceInfo.lastUpdate = millis();
    }
}

// Публичные методы
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

void WebastoInfo::getAllInfo()
{
    getWBusVersion();
    getDeviceName();
    getWBusCode();
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