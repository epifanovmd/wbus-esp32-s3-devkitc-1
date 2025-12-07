// src/common/Constants.h
#pragma once
#include <Arduino.h>
#include <HardwareSerial.h>

// Конфигурация Webasto W-Bus
constexpr uint8_t TXHEADER = 0xF4;  // WTT -> Нагреватель
constexpr uint8_t RXHEADER = 0x4F;  // Нагреватель -> WTT 

// RGB LED
constexpr int RGB_PIN = LED_BUILTIN;