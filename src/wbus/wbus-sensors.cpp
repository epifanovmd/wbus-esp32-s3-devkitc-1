#include "common/utils/utils.h"
#include "wbus/wbus-sensors.h"

WebastoSensors webastoSensors;

void WebastoSensors::handleOperationalInfoResponse(bool success, String cmd, String response)
{
  if (success)
  {
    int byteCount = 0;
    byte data[20];
    String cleanData = response;
    cleanData.replace(" ", "");

    for (int i = 0; i < cleanData.length(); i += 2)
    {
      if (byteCount < 20)
      {
        data[byteCount++] = hexStringToByte(cleanData.substring(i, i + 2));
      }
    }

    // Проверяем что это ответ на команду 0x50 индекс 0x05
    if (byteCount >= 13 && data[2] == 0xD0 && data[3] == 0x05)
    {
      // Данные начинаются с 4-го байта
      operationalMeasurements.temperature = data[4] - 50.0;                         // 09 (Offset +50°C)
      operationalMeasurements.voltage = (float)((data[5] << 8) | data[6]) / 1000.0; // 2F 8A
      operationalMeasurements.flameDetected = (data[7] == 0x01);                    // 00
      operationalMeasurements.heatingPower = (data[8] << 8) | data[9];              // 00 00
      operationalMeasurements.flameResistance = (data[10] << 8) | data[11];         // 03 E8
    }

    Serial.println();
    Serial.println("═══════════════════════════════════════════════════════════");
    Serial.println("         📊 ДАННЫЕ ДАТЧИКОВ                                ");
    Serial.println("═══════════════════════════════════════════════════════════");
    Serial.printf("🌡️  Температура:      %6.1f °C\n", operationalMeasurements.temperature);
    Serial.printf("🔋 Напряжение:        %6.1f V\n", operationalMeasurements.voltage);
    Serial.printf("🔥 Мощность:          %6d W\n", operationalMeasurements.heatingPower);
    Serial.printf("🔍 Сопротивление:     %6d мОм\n", operationalMeasurements.flameResistance);
    Serial.printf("🔄 Пламя:             %14s\n", operationalMeasurements.flameDetected ? "Обнаружено" : "Отсутствует");
    Serial.println();
  }
}

void WebastoSensors::handleFuelSettingsResponse(bool success, String cmd, String response)
{
    if (!success)
    {
        Serial.println("❌ Ошибка чтения настроек топлива");
        return;
    }

    // Парсим ответ
    int byteCount = 0;
    byte data[20];
    String cleanData = response;
    cleanData.replace(" ", "");

    for (int i = 0; i < cleanData.length(); i += 2)
    {
        if (byteCount < 20)
        {
            data[byteCount++] = hexStringToByte(cleanData.substring(i, i + 2));
        }
    }

    // Проверяем структуру пакета: 4f 06 d0 04 [type] [max_time] [vent_factor] [crc]
    if (byteCount >= 7 && data[2] == 0xD0 && data[3] == 0x04)
    {
        // Извлекаем данные (байты 4, 5, 6)
        fuelSettings.fuelType = data[4];
        fuelSettings.maxHeatingTime = data[5];
        fuelSettings.ventilationFactor = data[6];
        
        // ОБНОВЛЕННОЕ определение типа топлива на основе реальных данных
        switch (fuelSettings.fuelType)
        {
            case 0x0D:
                fuelSettings.fuelTypeName = "Дизельное топливо";
                break;
            case 0x1D:
                fuelSettings.fuelTypeName = "Дизельное топливо (альтернативный код)";
                break;
            case 0x2D:
                fuelSettings.fuelTypeName = "Бензин";
                break;
            case 0x03:
                fuelSettings.fuelTypeName = "Газ";
                break;
            case 0x05:
                fuelSettings.fuelTypeName = "Биотопливо";
                break;
            default:
                // Попробуем определить по диапазонам
                if (fuelSettings.fuelType >= 0x01 && fuelSettings.fuelType <= 0x0F)
                    fuelSettings.fuelTypeName = "Дизельные топлива";
                else if (fuelSettings.fuelType >= 0x10 && fuelSettings.fuelType <= 0x2F)
                    fuelSettings.fuelTypeName = "Бензины";
                else if (fuelSettings.fuelType >= 0x30 && fuelSettings.fuelType <= 0x4F)
                    fuelSettings.fuelTypeName = "Газовые топлива";
                else
                    fuelSettings.fuelTypeName = "Неизвестный тип";
                break;
        }

        // Выводим информацию
        Serial.println();
        Serial.println("═══════════════════════════════════════════════════════════");
        Serial.println("         ⛽ НАСТРОЙКИ ТОПЛИВА                              ");
        Serial.println("═══════════════════════════════════════════════════════════");
        Serial.println("Тип топлива:           " + fuelSettings.fuelTypeName);
        Serial.println("Код типа:              0x" + String(fuelSettings.fuelType, HEX) + 
                      " (" + String(fuelSettings.fuelType, DEC) + ")");
        Serial.println("Макс. время нагрева:   " + String(fuelSettings.maxHeatingTime) + " минут");
        
        // Детальная интерпретация коэффициента вентиляции
        String ventInfo = "0x" + String(fuelSettings.ventilationFactor, HEX);
        String ventDescription = "";
        
        switch (fuelSettings.ventilationFactor) {
            case 0x3C:
                ventDescription = "стандартный (60 мин)";
                break;
            case 0x1E:
                ventDescription = "сокращенный (30 мин)";
                break;
            case 0x0A:
                ventDescription = "минимальный (10 мин)";
                break;
            case 0x5A:
                ventDescription = "увеличенный (90 мин)";
                break;
            default:
                ventDescription = "пользовательский (" + String(fuelSettings.ventilationFactor) + " мин)";
                break;
        }
        
        Serial.println("Коэф. вентиляции:      " + ventInfo + " - " + ventDescription);
        
        // Дополнительная информация о конфигурации
        Serial.println();
        Serial.println("💡 ИНТЕРПРЕТАЦИЯ:");
        Serial.println("   • Устройство настроено на " + fuelSettings.fuelTypeName);
        Serial.println("   • Максимальное время работы: " + String(fuelSettings.maxHeatingTime) + " минут");
        Serial.println("   • Время вентиляции: " + String(fuelSettings.ventilationFactor) + " минут");
        
        // Проверка типичных конфигураций
        if (fuelSettings.fuelType == 0x0D && fuelSettings.maxHeatingTime == 60 && fuelSettings.ventilationFactor == 60) {
            Serial.println("   ✅ Стандартная конфигурация для дизельного топлива");
        }
        
        Serial.println();
    }
    else
    {
         Serial.println();
        Serial.println("❌ Неверный формат пакета настроек топлива");
        Serial.println("   Ожидалось: 4f 06 d0 04 [type] [max_time] [vent_factor] [crc]");
        Serial.println("   Получено: " + response);
    }
}

void WebastoSensors::getOperationalInfo()
{
  wbusQueue.add(CMD_READ_SENSOR_OPERATIONAL, [this](bool success, String cmd, String response)
                { this->handleOperationalInfoResponse(success, cmd, response); });
}

void WebastoSensors::getFuelSettings()
{
  wbusQueue.add(CMD_READ_SENSOR_FUEL_SETTINGS,
                [this](bool success, String cmd, String response)
                {
                  this->handleFuelSettingsResponse(success, cmd, response);
                });
}