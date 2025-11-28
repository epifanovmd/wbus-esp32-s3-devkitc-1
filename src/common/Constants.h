// src/common/Constants.h
#pragma once
#include <Arduino.h>
#include <HardwareSerial.h>

// Пины для управления TJA1020
constexpr int NSLP_PIN = 7;     // Sleep control (active LOW)
constexpr int NWAKE_PIN = 6;    // Wake-up input (active LOW) 
constexpr int RXD_PULLUP = 8;   // Пин для подтяжки RXD (open-drain)

// Конфигурация Webasto W-Bus
constexpr uint8_t TXHEADER = 0xF4;  // WTT -> Нагреватель
constexpr uint8_t RXHEADER = 0x4F;  // Нагреватель -> WTT 
constexpr int MESSAGE_BUFFER_SIZE = 32;
constexpr int BAUD_RATE = 2400;
constexpr int SERIAL_CONFIG = SERIAL_8E1;  // 8 бит, Even parity, 1 стоп-бит
constexpr int RX_TJA_PIN = 18;
constexpr int TX_TJA_PIN = 17;

// RGB LED
constexpr int RGB_PIN = LED_BUILTIN;

// Глобальный serial для W-Bus
extern HardwareSerial KLineSerial;