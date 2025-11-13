// tja1020.cpp

#include <HardwareSerial.h>
#include "common/constants.h"
#include "common/serial/serial.h"
#include "tja1020.h"

void initTJA1020() {
    // Инициализация пинов управления TJA1020
  pinMode(NSLP_PIN, OUTPUT);
  pinMode(NWAKE_PIN, OUTPUT);
  pinMode(RXD_PULLUP, OUTPUT);
  
  // Подтяжка RXD к 3.3V
  digitalWrite(RXD_PULLUP, HIGH);
  
  // Изначально спящий режим
  digitalWrite(NSLP_PIN, LOW);
  digitalWrite(NWAKE_PIN, HIGH);
}

// Функция для пробуждения TJA1020
void wakeUpTJA1020() {
  Serial.println("Пробуждение TJA1020...");
  
  digitalWrite(NSLP_PIN, HIGH);
  delay(10);
  
  digitalWrite(NWAKE_PIN, LOW);
  delay(2);
  digitalWrite(NWAKE_PIN, HIGH);
  
  delay(50);
  
  // Инициализируем UART для W-Bus (2400 8E1)
  WBusSerial.begin(BAUD_RATE, SERIAL_CONFIG, 18, 17);
  
  Serial.println("✅ TJA1020 готов к работе на 2400 8E1");
}

// Функция для перевода TJA1020 в спящий режим
void sleepTJA1020() {
  Serial.println("Перевод TJA1020 в спящий режим...");
  
  digitalWrite(17, HIGH);
  delay(10);
  
  WBusSerial.end();
  digitalWrite(NSLP_PIN, LOW);
  digitalWrite(NWAKE_PIN, HIGH);
  
  delay(10);
  Serial.println("✅ TJA1020 в спящем режиме");
}