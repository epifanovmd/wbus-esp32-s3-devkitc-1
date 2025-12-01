// src/infrastructure/protocol/WBusInfoDecoder.h
#pragma once
#include <Arduino.h>
#include "../../domain/Entities.h"
#include "../../common/Utils.h"

class WBusInfoDecoder {
public:
    static String decodeWBusVersion(const String& response) {
        String result = "";
        
        String versionData = extractDataFromResponse(response, "d10a", 2);
        if (versionData.length() == 2) {
            uint8_t versionByte = Utils::hexStringToByte(versionData);

            result = String((versionByte >> 4) & 0x0F) + "." + String(versionByte & 0x0F);
        }
        
        return result;
    }
    
    static String decodeDeviceName(const String& response) {
        String result = "";
        
        String nameData = extractDataFromResponse(response, "d10b");
        if (nameData.length() > 0) {
            String text = hexToString(nameData);
            text.trim();
            result = text;
        }
        
        return result;
    }
    
    static DecodedWBusCode decodeWBusCode(const String& response) {
        DecodedWBusCode result = {"", ""};
        
        String codeData = extractDataFromResponse(response, "d10c");
        if (codeData.length() > 0) {
            result.codeString = codeData;
            result.supportedFunctions = analyzeWBusCode(codeData);
        }
        
        return result;
    }
    
    static String decodeDeviceID(const String& response) {
        String result = "";
        
        String idData = extractDataFromResponse(response, "d101");
        if (idData.length() > 0) {
            result = idData;
        }
        
        return result;
    }
    
    static DecodedManufactureDate decodeHeaterManufactureDate(const String& response) {
        return decodeManufactureDate(response, "d105");
    }
    
    static DecodedManufactureDate decodeControllerManufactureDate(const String& response) {
        return decodeManufactureDate(response, "d104");
    }
    
    static String decodeCustomerID(const String& response) {
        String result = "";
        
        String data = extractDataFromResponse(response, "d107");
        if (data.length() > 0) {
            result = hexToString(data);
        }
        
        return result;
    }
    
    static String decodeSerialNumber(const String& response) {
        String result = "";
        
        String data = extractDataFromResponse(response, "d109");
        if (data.length() >= 10) {
            result = data.substring(0, 10);
        }
        
        return result;
    }

private:
    static String extractDataFromResponse(const String& response, const String& command, int dataLength = 0) {
        String cleanData = response;
        cleanData.replace(" ", "");
        cleanData.toLowerCase();
        
        int start = cleanData.indexOf(command);
        if (start == -1) {
            return "";
        }
        
        start += command.length();
        
        if (dataLength > 0) {
            if (cleanData.length() >= start + dataLength) {
                return cleanData.substring(start, start + dataLength);
            }
        } else {
            if (cleanData.length() >= start + 2) {
                return cleanData.substring(start, cleanData.length() - 2);
            }
        }
        
        return "";
    }
    
    static String hexToString(const String& hexData) {
        String result = "";
        for (int i = 0; i < hexData.length(); i += 2) {
            if (i + 2 <= hexData.length()) {
                String byteStr = hexData.substring(i, i + 2);
                if (byteStr == "00") break;
                char c = (char)strtoul(byteStr.c_str(), NULL, 16);
                if (c >= 32 && c <= 126) {
                    result += c;
                }
            }
        }
        return result;
    }
    
    static DecodedManufactureDate decodeManufactureDate(const String& response, const String& command) {
        DecodedManufactureDate result = {"", 0, 0, 0};
        
        String dateData = extractDataFromResponse(response, command, 6);
        if (dateData.length() == 6) {
            String dayStr = dateData.substring(0, 2);
            String monthStr = dateData.substring(2, 4);
            String yearStr = dateData.substring(4, 6);
            
            result.day = Utils::hexStringToByte(dayStr);
            result.month = Utils::hexStringToByte(monthStr);
            result.year = 2000 + Utils::hexStringToByte(yearStr);
            result.dateString = dayStr + "." + monthStr + ".20" + yearStr;
        }
        
        return result;
    }
    
    static String analyzeWBusCode(const String& codeData) {
        String functions = "";
        
        for (int byteNum = 0; byteNum < 7; byteNum++) {
            if (byteNum * 2 + 2 <= codeData.length()) {
                String byteStr = codeData.substring(byteNum * 2, byteNum * 2 + 2);
                uint8_t byteVal = Utils::hexStringToByte(byteStr);
                functions += analyzeWBusCodeByte(byteNum, byteVal);
            }
        }
        
        return functions;
    }
    
    static String analyzeWBusCodeByte(uint8_t byteNum, uint8_t byteVal) {
        String result = "";

        switch (byteNum) {
            case 0: // Byte 0
                if (byteVal & 0x08) result += "Простое вкл/выкл, ";
                if (byteVal & 0x10) result += "Паркинг-нагрев, ";
                if (byteVal & 0x20) result += "Дополнительный нагрев, ";
                if (byteVal & 0x40) result += "Вентиляция, ";
                if (byteVal & 0x80) result += "Boost режим, ";
                break;

            case 1: // Byte 1
                if (byteVal & 0x02) result += "Внешнее управление цирк. насосом, ";
                if (byteVal & 0x04) result += "Вентилятор горения (CAV), ";
                if (byteVal & 0x08) result += "Свеча накаливания, ";
                if (byteVal & 0x10) result += "Топливный насос (FP), ";
                if (byteVal & 0x20) result += "Циркуляционный насос (CP), ";
                if (byteVal & 0x40) result += "Реле вентилятора автомобиля (VFR), ";
                if (byteVal & 0x80) result += "Желтый LED, ";
                break;

            case 2: // Byte 2
                if (byteVal & 0x01) result += "Зеленый LED, ";
                if (byteVal & 0x02) result += "Искровой разрядник (нет свечи), ";
                if (byteVal & 0x04) result += "Соленоидный клапан, ";
                if (byteVal & 0x08) result += "Индикатор вспомогательного привода, ";
                if (byteVal & 0x10) result += "Сигнал генератора D+, ";
                if (byteVal & 0x20) result += "Вентилятор в RPM (не в %), ";
                break;

            case 3: // Byte 3
                if (byteVal & 0x02) result += "Калибровка CO2, ";
                if (byteVal & 0x08) result += "Индикатор работы (OI), ";
                break;

            case 4: // Byte 4
                if (byteVal & 0x10) result += "Мощность в ваттах (иначе в %), ";
                if (byteVal & 0x40) result += "Индикатор пламени (FI), ";
                if (byteVal & 0x80) result += "Подогрев форсунки, ";
                break;

            case 5: // Byte 5
                if (byteVal & 0x20) result += "Флаг зажигания (T15), ";
                if (byteVal & 0x40) result += "Доступны температурные пороги, ";
                if (byteVal & 0x80) result += "Чтение подогрева топлива, ";
                break;

            case 6: // Byte 6
                if (byteVal & 0x02) result += "  • Уставки: сопр. пламени, об. вентилятора, темп. выхода\n";
                break;
        }

        return result;
    }
};