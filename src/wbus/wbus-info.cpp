#include "wbus-info.h"

WebastoInfo webastoInfo;

void WebastoInfo::getWBusVersion()
{
    wbusQueue.add(CMD_READ_INFO_WBUS_VERSION, [](bool status, String tx, String rx)
                  {
      if (!status) {
            Serial.println("Ошибка связи");
            return;
        }
        
        rx.replace(" ", "");
        rx.toLowerCase();
        
        // Извлекаем данные после "d10a" (команда 0x51 + ACK, индекс 0x0A)
        int start = rx.indexOf("d10a") + 4;
        if (start >= 4 && rx.length() >= start + 2) {
            String versionData = rx.substring(start, start + 2);
            
            // Версия в формате: старший ниббл.младший ниббл
            uint8_t versionByte = strtoul(versionData.c_str(), NULL, 16);
            uint8_t major = (versionByte >> 4) & 0x0F;
            uint8_t minor = versionByte & 0x0F;
            
            Serial.println();
            Serial.print("Версия W-шины: " + String(major) + "." + String(minor));
        } else {
                   Serial.println();
            Serial.print("Ошибка извлечения версии");
        } });
}

void WebastoInfo::getDeviceName()
{
    wbusQueue.add(CMD_READ_INFO_DEVICE_NAME, [](bool status, String tx, String rx)
                  {

      if (!status) {
            Serial.println("Ошибка связи");
            return;
        }
        
        rx.replace(" ", "");
        rx.toLowerCase();
        
        // Извлекаем данные после "d10b" (команда 0x51 + ACK, индекс 0x0B)
        int start = rx.indexOf("d10b") + 4;
        if (start >= 4) {
            String nameData = rx.substring(start, rx.length() - 2);
            
            // Конвертируем HEX в ASCII
            String deviceName = "";
            for (int i = 0; i < nameData.length(); i += 2) {
                if (i + 2 <= nameData.length()) {
                    String byteStr = nameData.substring(i, i + 2);
                    char c = (char)strtoul(byteStr.c_str(), NULL, 16);
                    if (c >= 32 && c <= 126) { // Печатные символы
                        deviceName += c;
                    }
                }
            }
            
                 Serial.println();
            Serial.print("Обозначение устройства: " + deviceName);
        } else {
                 Serial.println();
            Serial.print("Ошибка извлечения имени");
        } });
}

String analyzeWBusCodeByte(uint8_t byteNum, uint8_t byteVal)
{
    String result = "";

    switch (byteNum)
    {
    case 0: // Byte 0
        if (byteVal & 0x01)
            result += "  • Unknown (ZH)\n";
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
        if (byteVal & 0x40)
            result += "  • Unknown (ZH)\n";
        if (byteVal & 0x80)
            result += "  • Unknown (ZH)\n";
        break;

    case 3: // Byte 3
        if (byteVal & 0x02)
            result += "  • Калибровка CO2\n";
        if (byteVal & 0x08)
            result += "  • Индикатор работы (OI)\n";
        break;

    case 4: // Byte 4
        if (byteVal & 0x0F)
            result += "  • Unknown (ZH)\n";
        if (byteVal & 0x10)
            result += "  • Мощность в ваттах (иначе в %)\n";
        if (byteVal & 0x20)
            result += "  • Unknown (ZH)\n";
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

void WebastoInfo::getWBusCode()
{
    wbusQueue.add(CMD_READ_INFO_WBUS_CODE, [](bool status, String tx, String rx)
                  {
        if (!status) {
            Serial.println("Ошибка связи");
            return;
        }
        
        rx.replace(" ", "");
        rx.toLowerCase();
        
        int start = rx.indexOf("d10c") + 4;
        if (start < 4) {
            Serial.println("Ошибка формата");
            return;
        }
        
        String codeData = rx.substring(start, rx.length() - 2);
        Serial.println();
        Serial.println("Кодирование W-шины: " + codeData);
        Serial.println("Поддерживаемые функции:");

        String result = "";
        
        // Анализ каждого байта
        for (int byteNum = 0; byteNum < 7; byteNum++) {
            if (byteNum * 2 + 2 <= codeData.length()) {
                String byteStr = codeData.substring(byteNum * 2, byteNum * 2 + 2);
                uint8_t byteVal = strtoul(byteStr.c_str(), NULL, 16);
                
                result += analyzeWBusCodeByte(byteNum, byteVal);
            }
        } 
    
        Serial.print(result); });
}

void WebastoInfo::getDeviceID()
{
    wbusQueue.add(CMD_READ_INFO_DEVICE_ID, [](bool status, String tx, String rx)
                  {
        
        if (!status) {
            Serial.println("Ошибка связи");
            return;
        }
        
        // Убираем пробелы и переводим в нижний регистр
        rx.replace(" ", "");
        rx.toLowerCase();
        
        // Просто выводим всё между "d101" и концом (исключая CRC)
        int start = rx.indexOf("d101") + 4;
        if (start >= 4) {
            String idData = rx.substring(start, rx.length() - 2);
            Serial.println();
            Serial.print("Идентификационный код блока управления: " + idData);
        } else {
            Serial.println();
            Serial.print("Ошибка формата пакета");
        } });
}

void WebastoInfo::getHeaterManufactureDate()
{
    wbusQueue.add(CMD_READ_INFO_HEATER_MFG_DATE, [](bool status, String tx, String rx)
                  {
        if (!status) {
            Serial.println("Ошибка связи");
            return;
        }
        
        rx.replace(" ", "");
        rx.toLowerCase();
        
        int start = rx.indexOf("d105") + 4;
        if (start >= 4 && rx.length() >= start + 6) {
            String dateData = rx.substring(start, start + 6);
            
            // Декодируем дату: DD MM YY
            String dayStr = dateData.substring(0, 2);
            String monthStr = dateData.substring(2, 4);
            String yearStr = dateData.substring(4, 6);
            
            Serial.println();
            Serial.println("Дата выпуска отопителя: " + String(dayStr) + "." + String(monthStr) + ".20" + String(yearStr));
        } });
}

void WebastoInfo::getControllerManufactureDate()
{
    wbusQueue.add(CMD_READ_INFO_CTRL_MFG_DATE, [](bool status, String tx, String rx)
                  {
        if (!status) {
            Serial.println("Ошибка связи");
            return;
        }
        
        rx.replace(" ", "");
        rx.toLowerCase();
        
        int start = rx.indexOf("d104") + 4;
        if (start >= 4 && rx.length() >= start + 6) {
            String dateData = rx.substring(start, start + 6);
            
            // Декодируем дату: DD MM YY
            String dayStr = dateData.substring(0, 2);
            String monthStr = dateData.substring(2, 4);
            String yearStr = dateData.substring(4, 6);
            
            Serial.println();
            Serial.println("Дата выпуска блока управления: " + String(dayStr) + "." + String(monthStr) + ".20" + String(yearStr));
        } });
}

void WebastoInfo::getCustomerID()
{
    wbusQueue.add(CMD_READ_INFO_CUSTOMER_ID, [](bool status, String tx, String rx)
                  {
        if (!status) {
            Serial.println("Ошибка связи");
            return;
        }
        
        rx.replace(" ", "");
        rx.toLowerCase();
        
        int start = rx.indexOf("d107") + 4;
        if (start >= 4) {
            String data = rx.substring(start, rx.length() - 2);
            
            // Умный поиск разделителя
            int zeroPos = -1;
            for (int i = 0; i < data.length(); i += 2) {
                if (i + 2 <= data.length()) {
                    String byteStr = data.substring(i, i + 2);
                    if (byteStr == "00") {
                        // Проверяем, что это разделитель, а не часть данных
                        bool isSeparator = true;
                        // Смотрим несколько байтов вперед (минимум 2 нулевых байта подряд)
                        int zeroCount = 0;
                        int maxCheck = i + 6; // Проверяем до 3 байтов вперед
                        if (maxCheck > data.length()) maxCheck = data.length();
                        
                        for (int j = i; j < maxCheck; j += 2) {
                            if (j + 2 <= data.length() && data.substring(j, j + 2) == "00") {
                                zeroCount++;
                            } else {
                                break;
                            }
                        }
                        // Считаем разделителем если нашли хотя бы 2 нулевых байта подряд
                        if (zeroCount >= 2) {
                            zeroPos = i;
                            break;
                        }
                    }
                }
            }
            
            String customerID = "";
            String additionalCode = "";
            
            if (zeroPos != -1) {
                // Часть до разделителя - Customer ID
                for (int i = 0; i < zeroPos; i += 2) {
                    if (i + 2 <= data.length()) {
                        String byteStr = data.substring(i, i + 2);
                        char c = (char)strtoul(byteStr.c_str(), NULL, 16);
                        if (c >= 32 && c <= 126) {
                            customerID += c;
                        }
                    }
                }
                
                // Часть после разделителя - дополнительный код
                int dataStart = zeroPos;
                // Пропускаем все нулевые байты разделителя
                while (dataStart < data.length() && 
                       dataStart + 2 <= data.length() && 
                       data.substring(dataStart, dataStart + 2) == "00") {
                    dataStart += 2;
                }
                
                // Читаем оставшиеся данные как дополнительный код
                for (int i = dataStart; i < data.length(); i += 2) {
                    if (i + 2 <= data.length()) {
                        String byteStr = data.substring(i, i + 2);
                        char c = (char)strtoul(byteStr.c_str(), NULL, 16);
                        if (c >= 32 && c <= 126) {
                            additionalCode += c;
                        }
                    }
                }
                
            } else {
                // Если разделитель не найден, пробуем прочитать все как Customer ID
                for (int i = 0; i < data.length(); i += 2) {
                    if (i + 2 <= data.length()) {
                        String byteStr = data.substring(i, i + 2);
                        if (byteStr != "00") {
                            char c = (char)strtoul(byteStr.c_str(), NULL, 16);
                            if (c >= 32 && c <= 126) {
                                customerID += c;
                            }
                        }
                    }
                }
            }
            
            Serial.println();
            Serial.println("Идентификационный код клиента: " + customerID);
            if (additionalCode.length() > 0) {
                Serial.println("Доп. код: " + additionalCode);
            }
        } });
}

void WebastoInfo::getSerialNumber()
{
    wbusQueue.add(CMD_READ_INFO_SERIAL_NUMBER, [](bool status, String tx, String rx)
                  {
        if (!status) {
            Serial.println("Ошибка связи");
            return;
        }
        
        rx.replace(" ", "");
        rx.toLowerCase();
        
        int start = rx.indexOf("d109") + 4;
        if (start >= 4) {
            String data = rx.substring(start, rx.length() - 2);
            String serialHex = data.substring(0, 10);
            String testStand = data.substring(10);
            
            Serial.println();
            Serial.println("Серийный номер: " + serialHex);
            Serial.println("Код испытательного стенда: " + testStand);
        } });
}