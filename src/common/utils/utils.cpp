// utils.cpp

#include "common/utils/utils.h"

// Функция для расчета контрольной суммы
byte calculateChecksum(byte* data, int length) {
  byte checksum = 0;
  for (int i = 0; i < length; i++) {
    checksum ^= data[i];
  }
  return checksum;
}

// Вспомогательная функция для проверки HEX строки
bool isHexString(String str) {
  for (unsigned int i = 0; i < str.length(); i++) {
    char c = str[i];
    if (!((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'))) {
      return false;
    }
  }
  return true;
}
