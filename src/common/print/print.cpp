// print.cpp

#include "print.h"
#include <HardwareSerial.h>

void printHelp()
{
  Serial.println("\n📋 КОМАНДЫ УПРАВЛЕНИЯ:");
  Serial.println("wake/w       - пробуждение TJA1020");
  Serial.println("sleep/s      - спящий режим TJA1020");
  Serial.println("connect/con  - подключение к Webasto");
  Serial.println("disconnect/dc- отключение от Webasto");
  Serial.println("status/st    - статус подключения");
  Serial.println("");
  Serial.println("🔥 КОМАНДЫ УПРАВЛЕНИЯ:");
  Serial.println("heat/h       - паркинг-нагрев 30 мин");
  Serial.println("vent/v       - вентиляция 30 мин");
  Serial.println("boost/b      - boost режим 15 мин");
  Serial.println("stop/off     - выключение нагревателя");
  Serial.println("pump on      - включить циркуляционный насос");
  Serial.println("pump off     - выключить циркуляционный насос");
  Serial.println("");
  Serial.println("🔧 ТЕСТИРОВАНИЕ КОМПОНЕНТОВ:");
  Serial.println("test fan     - тест вентилятора горения (5сек, 50%)");
  Serial.println("test fuel    - тест топливного насоса (3сек, 10Гц)");
  Serial.println("test glow    - тест свечи накаливания (5сек, 50%)");
  Serial.println("test circ    - тест циркуляционного насоса (5сек, 80%)");
  Serial.println("test vehicle - тест вентилятора авто (5сек)");
  Serial.println("test solenoid- тест соленоидного клапана (5сек)");
  Serial.println("test preheat - тест подогрева топлива (5сек, 50%)");
  Serial.println("");
  Serial.println("📊 ИНФОРМАЦИЯ:");
  Serial.println("info/i       - информация о Webasto");
  Serial.println("sensors      - данные датчиков");
  Serial.println("errors/err   - чтение ошибок");
  Serial.println("clear/clr    - стереть ошибки");
  Serial.println("help/h       - эта справка");
  Serial.println("========================================");
}

// Функция для красивого вывода HEX
void printHex(byte value, bool newLine)
{
  if (value < 0x10)
    Serial.print("0");
  Serial.print(value, HEX);
  if (newLine)
    Serial.println();
  else
    Serial.print(" ");
}