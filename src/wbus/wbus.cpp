// wbus.cpp

#include "wbus/wbus.h"

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
    wbusQueue.setProcessDelay(150);

    webastoInfo.getAllInfo();
    webastoSensors.getOperationalInfo();
    webastoSensors.getFuelSettings();
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