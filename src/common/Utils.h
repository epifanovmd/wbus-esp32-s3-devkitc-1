// src/common/Utils.h
#pragma once
#include <Arduino.h>

class Utils {
public:
    static byte calculateChecksum(byte *data, int length) {
        byte checksum = 0;
        for (int i = 0; i < length; i++) {
            checksum ^= data[i];
        }
        return checksum;
    }
    
    static bool isHexString(String str) {
        for (unsigned int i = 0; i < str.length(); i++) {
            char c = str[i];
            if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
                return false;
            }
        }
        return true;
    }
    
    static String byteToHexString(byte b) {
        return (b < 0x10) ? "0" + String(b, HEX) : String(b, HEX);
    }
    
    static byte hexStringToByte(const String &hexStr) {
        return (byte)strtol(hexStr.c_str(), NULL, 16);
    }
    
    static byte* hexStringToByteArray(const String &hexString, int &byteCount) {
        static byte data[32];
        byteCount = 0;
        
        String cleanString = hexString;
        cleanString.replace(" ", "");
        
        for (int i = 0; i < cleanString.length(); i += 2) {
            if (byteCount < 32 && i + 2 <= cleanString.length()) {
                data[byteCount++] = hexStringToByte(cleanString.substring(i, i + 2));
            }
        }
        
        return data;
    }
    
    static void printHex(byte value, bool newLine) {
        if (value < 0x10) Serial.print("0");
        Serial.print(value, HEX);
        if (newLine) Serial.println();
        else Serial.print(" ");
    }
};