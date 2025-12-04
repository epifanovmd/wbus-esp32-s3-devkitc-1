// src/common/Constants.h
#pragma once
#include <Arduino.h>
#include <HardwareSerial.h>

// Конфигурация Webasto W-Bus
constexpr uint8_t TXHEADER = 0xF4;  // WTT -> Нагреватель
constexpr uint8_t RXHEADER = 0x4F;  // Нагреватель -> WTT 
constexpr int MESSAGE_BUFFER_SIZE = 32;

// RGB LED
constexpr int RGB_PIN = LED_BUILTIN;

// Глобальный serial для W-Bus
extern HardwareSerial KLineSerial;