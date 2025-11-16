// wbus-sender.cpp

#include "wbus-sender.h"
#include "common/utils/utils.h"
#include "common/print/print.h"
#include "common/serial/serial.h"

// Функция проверки валидности WBUS пакета (обновленная)
bool validateWbusPacket(WBusPacket packet) {
  // Проверка минимальной длины
  if (packet.byteCount < 3) {
    Serial.println();
    Serial.println("❌ Слишком короткий пакет (минимум 3 байта)");
    return false;
  }

  // Проверка заголовка
  byte header = packet.data[0];
  if (header != TXHEADER && header != RXHEADER) {
    Serial.println();
    Serial.print("❌ Неверный заголовок: ");
    printHex(header, true);
    return false;
  }

  // Проверка длины пакета
  byte declaredLength = packet.data[1];
  if (packet.byteCount != declaredLength + 2) { // +2 для header и length байтов
    Serial.println();
    Serial.print("❌ Несоответствие длины: объявлено ");
    Serial.print(declaredLength);
    Serial.print(", фактически ");
    Serial.println(packet.byteCount - 2);
    return false;
  }

  // Проверка контрольной суммы
  byte calculatedChecksum = 0;
  for (int i = 0; i < packet.byteCount - 1; i++) {
    calculatedChecksum ^= packet.data[i];
  }

  byte receivedChecksum = packet.data[packet.byteCount - 1];

  if (calculatedChecksum != receivedChecksum) {
    Serial.println();
    Serial.print("❌ Контрольная сумма неверна!");
    Serial.print("   Ожидалось: ");
    printHex(calculatedChecksum, false);
    Serial.print(", получено: ");
    printHex(receivedChecksum, true);
    return false;
  }

  return true;
}

// Функция преобразования строки в массив байтов
WBusPacket parseHexStringToPacket(String input) {
  WBusPacket packet;
  packet.byteCount = 0;
  
  input.trim();
  input.toUpperCase();

  int startIndex = 0;
  int spaceIndex = input.indexOf(' ');

  while (spaceIndex != -1 && packet.byteCount < MESSAGE_BUFFER_SIZE) {
    String byteStr = input.substring(startIndex, spaceIndex);
    byteStr.trim();

    if (byteStr.length() > 0 && isHexString(byteStr)) {
      packet.data[packet.byteCount++] = (byte)strtol(byteStr.c_str(), NULL, 16);
    }

    startIndex = spaceIndex + 1;
    spaceIndex = input.indexOf(' ', startIndex);
  }

  // Последний байт
  if (startIndex < input.length() && packet.byteCount < MESSAGE_BUFFER_SIZE) {
    String byteStr = input.substring(startIndex);
    byteStr.trim();

    if (byteStr.length() > 0 && isHexString(byteStr)) {
      packet.data[packet.byteCount++] = (byte)strtol(byteStr.c_str(), NULL, 16);
    }
  }

  return packet;
}

// Единая функция отправки пакета (принимает строку)
bool sendWbusCommand(String command) {
  // Проверка режима TJA1020
  if (digitalRead(NSLP_PIN) == LOW) {
    Serial.println("❌ TJA1020 в спящем режиме!");
    return false;
  }

  WBusPacket packet = parseHexStringToPacket(command);
 
  if (packet.byteCount == 0) {
    Serial.println("❌ Неверный формат данных");
    return false;
  }

  // Валидация пакета
  if (!validateWbusPacket(packet)) {
    return false;
  }

  // Отправка пакета
  for (int i = 0; i < packet.byteCount; i++) {
    WBusSerial.write(packet.data[i]);
  }

  return true;
}