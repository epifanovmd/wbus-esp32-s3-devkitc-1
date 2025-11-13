// print.cpp

#include "print.h"
#include <HardwareSerial.h>

void printHelp() {
  Serial.println("\n📋 КОМАНДЫ УПРАВЛЕНИЯ:");
  Serial.println("wake/w       - пробуждение TJA1020");
  Serial.println("sleep/s      - спящий режим TJA1020");
  Serial.println("clear/c      - очистка экрана");
  Serial.println("connect/con  - подключение к Webasto");
  Serial.println("disconnect  - отключение от Webasto");
  Serial.println("errors/err   - чтение ошибок");
  Serial.println("help/h       - эта справка");
  Serial.println("=================================");
}

// Функция для красивого вывода HEX
void printHex(byte value, bool newLine) {
  if (value < 0x10) Serial.print("0");
  Serial.print(value, HEX);
  if (newLine) Serial.println();
  else Serial.print(" ");
}