#include "wbus-info-decoder.h"
#include "common/utils/utils.h"

WBusDecoder wBusDecoder;

// Вспомогательная функция: преобразование HEX в строку
String WBusDecoder::hexToString(const String& hexData) {
    String result = "";
    for (int i = 0; i < hexData.length(); i += 2) {
        if (i + 2 <= hexData.length()) {
            String byteStr = hexData.substring(i, i + 2);
            if (byteStr == "00") break; // Конец строки
            char c = (char)strtoul(byteStr.c_str(), NULL, 16);
            if (c >= 32 && c <= 126) {
                result += c;
            }
        }
    }
    return result;
}

// Вспомогательная функция: извлечение данных из ответа
String WBusDecoder::extractDataFromResponse(const String& response, const String& command, int dataLength) {
    String cleanData = response;
    cleanData.replace(" ", "");
    cleanData.toLowerCase();
    
    int start = cleanData.indexOf(command) + command.length();
    if (start < command.length()) {
        return ""; // Команда не найдена
    }
    
    if (dataLength > 0) {
        // Фиксированная длина данных
        if (cleanData.length() >= start + dataLength) {
            return cleanData.substring(start, start + dataLength);
        }
    } else {
        // Переменная длина данных (до конца, исключая CRC)
        if (cleanData.length() >= start + 2) { // Минимум 1 байт данных + 2 байта CRC
            return cleanData.substring(start, cleanData.length() - 2);
        }
    }
    
    return "";
}

// Декодирование версии W-Bus
DecodedVersion WBusDecoder::decodeWBusVersion(const String& response) {
    DecodedVersion result = {"", 0, 0, false};
    
    String versionData = extractDataFromResponse(response, "d10a", 2);
    if (versionData.length() == 2) {
        uint8_t versionByte = strtoul(versionData.c_str(), NULL, 16);
        result.major = (versionByte >> 4) & 0x0F;
        result.minor = versionByte & 0x0F;
        result.versionString = String(result.major) + "." + String(result.minor);
        result.isValid = true;
    }
    
    return result;
}

// Декодирование имени устройства
DecodedTextData WBusDecoder::decodeDeviceName(const String& response) {
    DecodedTextData result = {"", false};
    
    String nameData = extractDataFromResponse(response, "d10b");
    if (nameData.length() > 0) {
        String text = hexToString(nameData);
        text.trim();

        result.text = text;
        result.isValid = true;
    }
    
    return result;
}

// Анализ байта W-Bus кода
String WBusDecoder::analyzeWBusCodeByte(uint8_t byteNum, uint8_t byteVal) {
    String result = "";

    switch (byteNum) {
    case 0: // Byte 0
        if (byteVal & 0x08) result += "  • Простое вкл/выкл\n";
        if (byteVal & 0x10) result += "  • Паркинг-нагрев\n";
        if (byteVal & 0x20) result += "  • Дополнительный нагрев\n";
        if (byteVal & 0x40) result += "  • Вентиляция\n";
        if (byteVal & 0x80) result += "  • Boost режим\n";
        break;

    case 1: // Byte 1
        if (byteVal & 0x02) result += "  • Внешнее управление цирк. насосом\n";
        if (byteVal & 0x04) result += "  • Вентилятор горения (CAV)\n";
        if (byteVal & 0x08) result += "  • Свеча накаливания\n";
        if (byteVal & 0x10) result += "  • Топливный насос (FP)\n";
        if (byteVal & 0x20) result += "  • Циркуляционный насос (CP)\n";
        if (byteVal & 0x40) result += "  • Реле вентилятора автомобиля (VFR)\n";
        if (byteVal & 0x80) result += "  • Желтый LED\n";
        break;

    case 2: // Byte 2
        if (byteVal & 0x01) result += "  • Зеленый LED\n";
        if (byteVal & 0x02) result += "  • Искровой разрядник (нет свечи)\n";
        if (byteVal & 0x04) result += "  • Соленоидный клапан\n";
        if (byteVal & 0x08) result += "  • Индикатор вспомогательного привода\n";
        if (byteVal & 0x10) result += "  • Сигнал генератора D+\n";
        if (byteVal & 0x20) result += "  • Вентилятор в RPM (не в %)\n";
        break;

    case 3: // Byte 3
        if (byteVal & 0x02) result += "  • Калибровка CO2\n";
        if (byteVal & 0x08) result += "  • Индикатор работы (OI)\n";
        break;

    case 4: // Byte 4
        if (byteVal & 0x10) result += "  • Мощность в ваттах (иначе в %)\n";
        if (byteVal & 0x40) result += "  • Индикатор пламени (FI)\n";
        if (byteVal & 0x80) result += "  • Подогрев форсунки\n";
        break;

    case 5: // Byte 5
        if (byteVal & 0x20) result += "  • Флаг зажигания (T15)\n";
        if (byteVal & 0x40) result += "  • Доступны температурные пороги\n";
        if (byteVal & 0x80) result += "  • Чтение подогрева топлива\n";
        break;

    case 6: // Byte 6
        if (byteVal & 0x02) result += "  • Уставки: сопр. пламени, об. вентилятора, темп. выхода\n";
        break;
    }

    return result;
}

// Декодирование W-Bus кода
DecodedWBusCode WBusDecoder::decodeWBusCode(const String& response) {
    DecodedWBusCode result = {"", "", false};
    
    String codeData = extractDataFromResponse(response, "d10c");
    if (codeData.length() > 0) {
        result.codeString = codeData;
        
        String functions = "";
        for (int byteNum = 0; byteNum < 7; byteNum++) {
            if (byteNum * 2 + 2 <= codeData.length()) {
                String byteStr = codeData.substring(byteNum * 2, byteNum * 2 + 2);
                uint8_t byteVal = strtoul(byteStr.c_str(), NULL, 16);
                functions += analyzeWBusCodeByte(byteNum, byteVal);
            }
        }
        
        result.supportedFunctions = functions;
        result.isValid = true;
    }
    
    return result;
}

// Декодирование ID устройства
DecodedTextData WBusDecoder::decodeDeviceID(const String& response) {
    DecodedTextData result = {"", false};
    
    String idData = extractDataFromResponse(response, "d101");
    if (idData.length() > 0) {
        result.text = idData;
        result.isValid = true;
    }
    
    return result;
}

// Общая функция декодирования даты производства
DecodedManufactureDate WBusDecoder::decodeManufactureDate(const String& response, const String& command) {
    DecodedManufactureDate result = {"", 0, 0, 0, false};
    
    String dateData = extractDataFromResponse(response, command, 6);
    if (dateData.length() == 6) {
        String dayStr = dateData.substring(0, 2);
        String monthStr = dateData.substring(2, 4);
        String yearStr = dateData.substring(4, 6);
        
        result.day = strtoul(dayStr.c_str(), NULL, 16);
        result.month = strtoul(monthStr.c_str(), NULL, 16);
        result.year = 2000 + strtoul(yearStr.c_str(), NULL, 16); // Предполагаем 2000+ годы
        result.dateString = dayStr + "." + monthStr + ".20" + yearStr;
        result.isValid = true;
    }
    
    return result;
}

// Декодирование даты производства нагревателя
DecodedManufactureDate WBusDecoder::decodeHeaterManufactureDate(const String& response) {
    return decodeManufactureDate(response, "d105");
}

// Декодирование даты производства контроллера
DecodedManufactureDate WBusDecoder::decodeControllerManufactureDate(const String& response) {
    return decodeManufactureDate(response, "d104");
}

// Декодирование Customer ID
DecodedTextData WBusDecoder::decodeCustomerID(const String& response) {
    DecodedTextData result = {"", false};
    
    String data = extractDataFromResponse(response, "d107");
    if (data.length() > 0) {
        result.text = hexToString(data);
        result.isValid = true;
    }
    
    return result;
}

// Декодирование серийного номера
DecodedTextData WBusDecoder::decodeSerialNumber(const String& response) {
    DecodedTextData result = {"", false};
    
    String data = extractDataFromResponse(response, "d109");
    if (data.length() >= 10) {
        result.text = data.substring(0, 10); // Серийный номер
        // testStandCode можно получить как data.substring(10) если нужно
        result.isValid = true;
    }
    
    return result;
}