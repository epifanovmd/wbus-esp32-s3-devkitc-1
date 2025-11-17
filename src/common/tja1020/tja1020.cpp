// tja1020.cpp

#include <HardwareSerial.h>
#include "common/constants.h"
#include "common/serial/serial.h"
#include "tja1020.h"

void initTJA1020()
{
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
void wakeUpTJA1020()
{
  digitalWrite(NSLP_PIN, HIGH);
  delay(10);

  digitalWrite(NWAKE_PIN, LOW);
  delay(2);
  digitalWrite(NWAKE_PIN, HIGH);

  delay(50);

  // Инициализируем UART для W-Bus (2400 8E1)
  KLineSerial.begin(BAUD_RATE, SERIAL_CONFIG, RX_TJA_PIN, TX_TJA_PIN);
}

// Функция для перевода TJA1020 в спящий режим
void sleepTJA1020()
{
  digitalWrite(TX_TJA_PIN, HIGH);
  delay(10);

  KLineSerial.end();
  digitalWrite(NSLP_PIN, LOW);
  digitalWrite(NWAKE_PIN, HIGH);

  delay(10);
}