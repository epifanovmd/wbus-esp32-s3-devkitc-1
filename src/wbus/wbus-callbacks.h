#include <Arduino.h>
#include "wbus/wbus.constants.h"

// Функция для сбора полной информации об устройстве
void collectFullDeviceInfo();

// Колбэк для сбора информации по частям  
void deviceInfoCollectorCallback(bool success, String cmd, String response);

// Функция вывода сводной информации
void printDeviceSummary();