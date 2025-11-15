// wbus.cpp

#include "wbus/wbus.h"

void operationalMeasurementsCallback(bool success, String cmd, String response)
{
  if (success)
  {
    OperationalMeasurements measurements = WBusDecoders::decodeOperationalMeasurements(response);

    Serial.println();
    Serial.println("┌──────────────────────────────────┐");
    Serial.println("│           📊 ДАННЫЕ ДАТЧИКА      │");
    Serial.println("├──────────────────────────────────┤");
    Serial.printf("│ 🌡️  Температура: %6.1f °C       │\n", measurements.temperature);
    Serial.printf("│ 🔋 Напряжение:   %6.1f V        │\n", measurements.voltage);
    Serial.printf("│ 🔥 Мощность:    %6d W         │\n", measurements.heatingPower);
    Serial.printf("│ 🔍 Сопротивление:%6d мОм      │\n", measurements.flameResistance);
    Serial.printf("│ 🔄 Пламя:       %14s      │\n", measurements.flameDetected ? "Обнаружено" : "Отсутствует");
    Serial.println("└──────────────────────────────────┘");
  }
  else
  {
    Serial.println("❌ Ошибка получения операционных измерений");
  }
}

void wakeUpWebasto()
{
  Serial.println("🔔 Пробуждение Webasto...");

  // BREAK set - удерживаем линию в LOW 50ms
  WBusSerial.write(0x00); // Отправляем 0 для создания BREAK
  delay(50);

  // BREAK reset - отпускаем линию и ждем 50ms
  WBusSerial.flush(); // Очищаем буфер
  delay(50);

  Serial.println("✅ BREAK последовательность отправлена");
  Serial.println("Webasto должен быть готов к работе");
}

void connectCallback(bool success, String cmd, String response)
{
  Serial.println();

  if (success)
  {
    Serial.print("✅ Подключение прошло успешно");
    // wbusQueue.setProcessDelay(150);
    wbusQueue.setProcessDelay(550);

    // collectFullDeviceInfo();
    wbusQueue.add(CMD_READ_SENSOR_OPERATIONAL, operationalMeasurementsCallback, true);
    //  for (int i = 0; i < SENSOR_COMMANDS_COUNT; i++) {
    //           sendWbusCommandWithAck(SENSOR_COMMANDS[i], nullptr, true);
    //         }
  }
  else
  {
    Serial.print("❌ Не удалось подключиться!");
  }
  Serial.println();
}

void connectToWebasto()
{
  wakeUpWebasto();

  delay(100);
  Serial.println("🔌 Подключение к Webasto...");

  for (int i = 0; i < INIT_COMMANDS_COUNT; i++)
  {
    if (i < INIT_COMMANDS_COUNT - 1)
    {
      wbusQueue.add(INIT_COMMANDS[i]);
    }
    else
    {
      wbusQueue.add(INIT_COMMANDS[i], connectCallback);
    }
  }
}

// Функция запроса конкретного датчика
void querySensor(byte sensorIndex)
{
  Serial.print("🔍 Запрос датчика 0x");

  // sendWbusPacket("");
}

// Функция запроса информации
void queryInfo(byte infoIndex)
{
  Serial.print("📋 Запрос информации 0x");

  // sendWbusPacket("");
}

// Функция чтения ошибок
void readErrors()
{
  Serial.println("⚠️ Чтение ошибок...");

  // byte errorData[] = {0x56, 0x01}; // Список ошибок
  // sendWbusPacket("");
}