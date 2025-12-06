// src/common/Constants.h
#pragma once
#include <Arduino.h>
#include <HardwareSerial.h>

constexpr const char *OTA_USERNAME = "admin";     // Изменить в production!
constexpr const char *OTA_PASSWORD = "Epifan123"; // Изменить в production!

// Конфигурация Webasto W-Bus
constexpr uint8_t TXHEADER = 0xF4;  // WTT -> Нагреватель
constexpr uint8_t RXHEADER = 0x4F;  // Нагреватель -> WTT 

// RGB LED
constexpr int RGB_PIN = LED_BUILTIN;

constexpr const char* FIRMWARE_VERSION = "1.0.2";