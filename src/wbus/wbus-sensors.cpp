#include "wbus-sensors.h"
#include "common/utils/utils.h"
#include "wbus-queue.h"
#include "wbus.constants.h"

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

    switch (fuelSettings.ventilationFactor)
    {
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

void WebastoSensors::handleOnOffFlagsResponse(bool success, String cmd, String response)
{
  if (!success)
  {
    Serial.println("❌ Ошибка чтения флагов подсистем");
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

  // Проверяем структуру пакета: 4f 04 d0 03 [flags] [crc]
  if (byteCount >= 5 && data[2] == 0xD0 && data[3] == 0x03)
  {
    // Извлекаем байт флагов (байт 4)
    byte flags = data[4];

    // Декодируем флаги согласно документации W-Bus
    onOffFlags.combustionAirFan = (flags & 0x01) != 0;   // Бит 0: Вентилятор горения
    onOffFlags.glowPlug = (flags & 0x02) != 0;           // Бит 1: Свеча накаливания
    onOffFlags.fuelPump = (flags & 0x04) != 0;           // Бит 2: Топливный насос
    onOffFlags.circulationPump = (flags & 0x08) != 0;    // Бит 3: Циркуляционный насос
    onOffFlags.vehicleFanRelay = (flags & 0x10) != 0;    // Бит 4: Реле вентилятора авто
    onOffFlags.nozzleStockHeating = (flags & 0x20) != 0; // Бит 5: Подогрев форсунки
    onOffFlags.flameIndicator = (flags & 0x40) != 0;     // Бит 6: Индикатор пламени

    // Формируем строку активных компонентов
    onOffFlags.activeComponents = "";
    if (onOffFlags.combustionAirFan)
      onOffFlags.activeComponents += "Вентилятор, ";
    if (onOffFlags.glowPlug)
      onOffFlags.activeComponents += "Свеча, ";
    if (onOffFlags.fuelPump)
      onOffFlags.activeComponents += "ТН, ";
    if (onOffFlags.circulationPump)
      onOffFlags.activeComponents += "ЦН, ";
    if (onOffFlags.vehicleFanRelay)
      onOffFlags.activeComponents += "Вент. авто, ";
    if (onOffFlags.nozzleStockHeating)
      onOffFlags.activeComponents += "Подогрев, ";
    if (onOffFlags.flameIndicator)
      onOffFlags.activeComponents += "Пламя, ";

    // Убираем последнюю запятую
    if (onOffFlags.activeComponents.length() > 0)
    {
      onOffFlags.activeComponents = onOffFlags.activeComponents.substring(0, onOffFlags.activeComponents.length() - 2);
    }
    else
    {
      onOffFlags.activeComponents = "нет активных";
    }

    // Выводим информацию
    Serial.println();
    Serial.println("═══════════════════════════════════════════════════════════");
    Serial.println("         🔧 СОСТОЯНИЕ ПОДСИСТЕМ                            ");
    Serial.println("═══════════════════════════════════════════════════════════");
    Serial.println("Байт флагов:           0x" + String(flags, HEX) + " (" + String(flags, DEC) + ")");
    Serial.println();

    // Детальный вывод состояния каждой подсистемы
    Serial.println("📋 СТАТУС КОМПОНЕНТОВ:");
    Serial.println("   " + String(onOffFlags.combustionAirFan ? "✅" : "❌") + " Вентилятор горения (CAF)");
    Serial.println("   " + String(onOffFlags.glowPlug ? "✅" : "❌") + " Свеча накаливания (GP)");
    Serial.println("   " + String(onOffFlags.fuelPump ? "✅" : "❌") + " Топливный насос (FP)");
    Serial.println("   " + String(onOffFlags.circulationPump ? "✅" : "❌") + " Циркуляционный насос (CP)");
    Serial.println("   " + String(onOffFlags.vehicleFanRelay ? "✅" : "❌") + " Реле вентилятора авто (VFR)");
    Serial.println("   " + String(onOffFlags.nozzleStockHeating ? "✅" : "❌") + " Подогрев форсунки (NSH)");
    Serial.println("   " + String(onOffFlags.flameIndicator ? "✅" : "❌") + " Индикатор пламени (FI)");
    Serial.println();
  }
  else
  {
    Serial.println("❌ Неверный формат пакета флагов подсистем");
    Serial.println("   Ожидалось: 4f 04 d0 03 [flags] [crc]");
    Serial.println("   Получено: " + response);
    Serial.println("   Длина: " + String(byteCount) + " байт");
  }
}

void WebastoSensors::handleStatusFlagsResponse(bool success, String cmd, String response)
{
  if (!success)
  {
    Serial.println("❌ Ошибка чтения статусных флагов");
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

  // Проверяем структуру пакета: 4f 07 d0 02 [byte0] [byte1] [byte2] [byte3] [byte4] [crc]
  if (byteCount >= 8 && data[2] == 0xD0 && data[3] == 0x02)
  {
    // Извлекаем байты статуса (байты 4-8)
    byte statusByte0 = data[4]; // Основные флаги
    byte statusByte1 = data[5]; // Сезон и управление
    byte statusByte2 = data[6]; // Генератор
    byte statusByte3 = data[7]; // Режимы
    byte statusByte4 = data[8]; // Зажигание

    // Декодируем байт 0 согласно документации W-Bus
    statusFlags.mainSwitch = (statusByte0 & 0x01) != 0;              // Бит 0: Главный выключатель
    statusFlags.supplementalHeatRequest = (statusByte0 & 0x10) != 0; // Бит 4: Доп. нагрев
    statusFlags.parkingHeatRequest = (statusByte0 & 0x20) != 0;      // Бит 5: Паркинг-нагрев
    statusFlags.ventilationRequest = (statusByte0 & 0x40) != 0;      // Бит 6: Вентиляция

    // Декодируем байт 1
    statusFlags.summerMode = (statusByte1 & 0x01) != 0;      // Бит 0: Летний режим
    statusFlags.externalControl = (statusByte1 & 0x02) != 0; // Бит 1: Внешнее управление

    // Декодируем байт 2
    statusFlags.generatorSignal = (statusByte2 & 0x10) != 0; // Бит 4: Генератор D+

    // Декодируем байт 3
    statusFlags.boostMode = (statusByte3 & 0x10) != 0;      // Бит 4: Boost режим
    statusFlags.auxiliaryDrive = (statusByte3 & 0x01) != 0; // Бит 0: Вспомогательный привод

    // Декодируем байт 4
    statusFlags.ignitionSignal = (statusByte4 & 0x01) != 0; // Бит 0: Зажигание T15

    // Формируем сводку статуса
    statusFlags.statusSummary = "";
    if (statusFlags.mainSwitch)
      statusFlags.statusSummary += "Включен, ";
    if (statusFlags.ignitionSignal)
      statusFlags.statusSummary += "Зажигание, ";
    if (statusFlags.generatorSignal)
      statusFlags.statusSummary += "Генератор, ";
    if (statusFlags.summerMode)
      statusFlags.statusSummary += "Лето, ";
    if (statusFlags.externalControl)
      statusFlags.statusSummary += "Внешнее упр., ";

    // Убираем последнюю запятую
    if (statusFlags.statusSummary.length() > 0)
    {
      statusFlags.statusSummary = statusFlags.statusSummary.substring(0, statusFlags.statusSummary.length() - 2);
    }
    else
    {
      statusFlags.statusSummary = "базовый статус";
    }

    // Определяем режим работы
    if (statusFlags.parkingHeatRequest)
    {
      statusFlags.operationMode = "🚗 Паркинг-нагрев";
    }
    else if (statusFlags.supplementalHeatRequest)
    {
      statusFlags.operationMode = "🔥 Дополнительный нагрев";
    }
    else if (statusFlags.ventilationRequest)
    {
      statusFlags.operationMode = "💨 Вентиляция";
    }
    else if (statusFlags.boostMode)
    {
      statusFlags.operationMode = "⚡ Boost режим";
    }
    else
    {
      statusFlags.operationMode = "💤 Ожидание";
    }

    // Выводим информацию
    Serial.println();
    Serial.println("═══════════════════════════════════════════════════════════");
    Serial.println("         📊 СТАТУСНЫЕ ФЛАГИ                               ");
    Serial.println("═══════════════════════════════════════════════════════════");
    Serial.println("Байты статуса:         " +
                   String(statusByte0, HEX) + " " +
                   String(statusByte1, HEX) + " " +
                   String(statusByte2, HEX) + " " +
                   String(statusByte3, HEX) + " " +
                   String(statusByte4, HEX));
    Serial.println();

    // Детальный вывод статусных флагов
    Serial.println("🎯 ОСНОВНОЙ СТАТУС:");
    Serial.println("   " + String(statusFlags.mainSwitch ? "✅" : "❌") + " Главный выключатель");
    Serial.println("   " + String(statusFlags.ignitionSignal ? "✅" : "❌") + " Зажигание (T15)");
    Serial.println("   " + String(statusFlags.generatorSignal ? "✅" : "❌") + " Генератор (D+)");
    Serial.println("   " + String(statusFlags.summerMode ? "✅" : "❌") + " Летний режим");
    Serial.println("   " + String(statusFlags.externalControl ? "✅" : "❌") + " Внешнее управление");

    Serial.println();
    Serial.println("🔥 ЗАПРОСЫ РЕЖИМОВ:");
    Serial.println("   " + String(statusFlags.parkingHeatRequest ? "✅" : "❌") + " Паркинг-нагрев");
    Serial.println("   " + String(statusFlags.supplementalHeatRequest ? "✅" : "❌") + " Доп. нагрев");
    Serial.println("   " + String(statusFlags.ventilationRequest ? "✅" : "❌") + " Вентиляция");
    Serial.println("   " + String(statusFlags.boostMode ? "✅" : "❌") + " Boost режим");
    Serial.println("   " + String(statusFlags.auxiliaryDrive ? "✅" : "❌") + " Вспомогательный привод");
    Serial.println();
  }
  else
  {
    Serial.println("❌ Неверный формат пакета статусных флагов");
    Serial.println("   Ожидалось: 4f 07 d0 02 [byte0] [byte1] [byte2] [byte3] [byte4] [crc]");
    Serial.println("   Получено: " + response);
    Serial.println("   Длина: " + String(byteCount) + " байт");
  }
}

String WebastoSensors::getStateName(uint8_t stateCode)
{
  switch (stateCode)
  {
  case 0x00:
    return "Продувка";
  case 0x01:
    return "Деактивация";
  case 0x02:
    return "Продувка ADR";
  case 0x03:
    return "Продувка рампы";
  case 0x04:
    return "Выключен";
  case 0x05:
    return "Горение частичная нагрузка";
  case 0x06:
    return "Горение полная нагрузка";
  case 0x07:
    return "Подача топлива";
  case 0x08:
    return "Запуск вентилятора";
  case 0x09:
    return "Прерывание топлива";
  case 0x0A:
    return "Диагностика";
  case 0x0B:
    return "Прерывание топливного насоса";
  case 0x0C:
    return "Измерение EMF";
  case 0x0D:
    return "Стабилизация";
  case 0x0E:
    return "Деактивация";
  case 0x0F:
    return "Опрос датчика пламени";
  case 0x10:
    return "Охлаждение датчика пламени";
  case 0x11:
    return "Фаза измерения датчика";
  case 0x12:
    return "Фаза измерения ZUE";
  case 0x13:
    return "Запуск вентилятора";
  case 0x14:
    return "Прогрев свечи";
  case 0x15:
    return "Блокировка нагрева";
  case 0x16:
    return "Инициализация";
  case 0x17:
    return "Компенсация пузырей";
  case 0x18:
    return "Холодный запуск вент.";
  case 0x19:
    return "Обогащение холодного пуска";
  case 0x1A:
    return "Охлаждение";
  case 0x1B:
    return "Смена нагрузки ЧН-ПН";
  case 0x1C:
    return "Вентиляция";
  case 0x1D:
    return "Смена нагрузки ПН-ЧН";
  case 0x1E:
    return "Новая инициализация";
  case 0x1F:
    return "Контролируемая работа";
  case 0x20:
    return "Контрольный период";
  case 0x21:
    return "Мягкий старт";
  case 0x22:
    return "Время безопасности";
  case 0x23:
    return "Продувка";
  case 0x24:
    return "Старт";
  case 0x25:
    return "Стабилизация";
  case 0x26:
    return "Стартовая рампа";
  case 0x27:
    return "Отключение питания";
  case 0x28:
    return "Блокировка";
  case 0x29:
    return "Блокировка ADR";
  case 0x2A:
    return "Время стабилизации";
  case 0x2B:
    return "Переход к контролю";
  case 0x2C:
    return "Состояние решения";
  case 0x2D:
    return "Предстартовая подача";
  case 0x2E:
    return "Накал";
  case 0x2F:
    return "Контроль мощности накала";
  case 0x30:
    return "Задержка снижения";
  case 0x31:
    return "Медленный запуск вент.";
  case 0x32:
    return "Дополнительный накал";
  case 0x33:
    return "Прерывание зажигания";
  case 0x34:
    return "Зажигание";
  case 0x35:
    return "Прерывистый накал";
  case 0x36:
    return "Мониторинг применения";
  case 0x37:
    return "Сохранение блокировки";
  case 0x38:
    return "Деактивация блокировки";
  case 0x39:
    return "Контроль выхода";
  case 0x3A:
    return "Управление цирк. насосом";
  case 0x3B:
    return "Инициализация µP";
  case 0x3C:
    return "Опрос паразитного света";
  case 0x3D:
    return "Предстарт";
  case 0x3E:
    return "Предзажигание";
  case 0x3F:
    return "Воспламенение";
  case 0x40:
    return "Стабилизация пламени";
  case 0x41:
    return "Горение паркинг-нагрев";
  case 0x42:
    return "Горение доп. нагрев";
  case 0x43:
    return "Сбой горения нагрев";
  case 0x44:
    return "Сбой горения доп. нагрев";
  case 0x45:
    return "Выключение после работы";
  case 0x46:
    return "Контроль после работы";
  case 0x47:
    return "После работы из-за сбоя";
  case 0x48:
    return "Время-контроль после сбоя";
  case 0x49:
    return "Блокировка цирк. насоса";
  case 0x4A:
    return "Контроль после паркинг";
  case 0x4B:
    return "Контроль после доп. нагрева";
  case 0x4C:
    return "Контроль с цирк. насосом";
  case 0x4D:
    return "Цирк. насос без нагрева";
  case 0x4E:
    return "Ожидание перенапряжения";
  case 0x4F:
    return "Обновление памяти ошибок";
  case 0x50:
    return "Цикл ожидания";
  case 0x51:
    return "Тест компонентов";
  case 0x52:
    return "Boost";
  case 0x53:
    return "Охлаждение";
  case 0x54:
    return "Постоянная блокировка";
  case 0x55:
    return "Холостой ход вент.";
  case 0x56:
    return "Отрыв";
  case 0x57:
    return "Опрос температуры";
  case 0x58:
    return "Предстарт пониженное напр.";
  case 0x59:
    return "Опрос аварии";
  case 0x5A:
    return "После работы соленоида";
  case 0x5B:
    return "Обновление ошибок соленоида";
  case 0x5C:
    return "Таймер после работы соленоида";
  case 0x5D:
    return "Попытка запуска";
  case 0x5E:
    return "Расширение предстарта";
  case 0x5F:
    return "Процесс горения";
  case 0x60:
    return "Таймер после работы пониж. напр.";
  case 0x61:
    return "Обновление ошибок перед выкл.";
  case 0x62:
    return "Рампа полной нагрузки";
  default:
    return "Неизвестное состояние";
  }
}

// Вспомогательная функция: получение описания состояния
String WebastoSensors::getStateDescription(uint8_t stateCode)
{
  // Группируем состояния по категориям
  if (stateCode == 0x04)
    return "Нагреватель выключен и готов к работе";
  if (stateCode >= 0x05 && stateCode <= 0x06)
    return "Активный процесс горения";
  if (stateCode >= 0x07 && stateCode <= 0x09)
    return "Фаза подачи топлива";
  if (stateCode >= 0x0E && stateCode <= 0x12)
    return "Работа с датчиком пламени";
  if (stateCode >= 0x13 && stateCode <= 0x15)
    return "Фаза запуска и прогрева";
  if (stateCode >= 0x1C && stateCode <= 0x1D)
    return "Режим вентиляции";
  if (stateCode >= 0x24 && stateCode <= 0x27)
    return "Процесс запуска";
  if (stateCode >= 0x2E && stateCode <= 0x35)
    return "Работа системы зажигания";
  if (stateCode >= 0x41 && stateCode <= 0x44)
    return "Основной процесс горения";
  if (stateCode >= 0x45 && stateCode <= 0x4D)
    return "Фаза завершения работы";
  if (stateCode >= 0x51 && stateCode <= 0x52)
    return "Специальные режимы";

  return "Промежуточное состояние системы";
}

// Вспомогательная функция: декодирование флагов состояния устройства
String WebastoSensors::decodeDeviceStateFlags(uint8_t flags)
{
  String result = "";

  if (flags & 0x01)
    result += "STFL, "; // Стартерный флаг
  if (flags & 0x02)
    result += "UEHFL, "; // Флаг верхнего предела температуры
  if (flags & 0x04)
    result += "SAFL, "; // Флаг безопасности
  if (flags & 0x08)
    result += "RZFL, "; // Флаг регулирования

  if (result.length() > 0)
  {
    result = result.substring(0, result.length() - 2);
  }
  else
  {
    result = "Нет флагов";
  }

  return result;
}

// НОВАЯ ФУНКЦИЯ: Обработка состояния работы
void WebastoSensors::handleOperatingStateResponse(bool success, String cmd, String response)
{
  if (!success)
  {
    Serial.println("❌ Ошибка чтения состояния работы");
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

  // Проверяем структуру пакета: 4f 0A d0 06 [state] [number] [flags] [unk1] [unk2] [unk3] [crc]
  if (byteCount >= 10 && data[2] == 0xD0 && data[3] == 0x06)
  {
    // Извлекаем данные состояния (байты 4-9)
    operatingState.stateCode = data[4];        // Код состояния
    operatingState.stateNumber = data[5];      // Номер состояния
    operatingState.deviceStateFlags = data[6]; // Флаги состояния устройства
    // data[7], data[8], data[9] - неизвестные байты

    // Получаем информацию о состоянии
    operatingState.stateName = getStateName(operatingState.stateCode);
    operatingState.stateDescription = getStateDescription(operatingState.stateCode);
    operatingState.deviceStateInfo = decodeDeviceStateFlags(operatingState.deviceStateFlags);

    // Выводим информацию
    Serial.println();
    Serial.println("═══════════════════════════════════════════════════════════");
    Serial.println("         🔄 СОСТОЯНИЕ РАБОТЫ                               ");
    Serial.println("═══════════════════════════════════════════════════════════");

    Serial.println("📊 ДАННЫЕ СОСТОЯНИЯ:");
    Serial.println("   Код состояния:      0x" + String(operatingState.stateCode, HEX) + " (" + String(operatingState.stateCode, DEC) + ")");
    Serial.println("   Номер состояния:    " + String(operatingState.stateNumber));
    Serial.println("   Флаги устройства:   0x" + String(operatingState.deviceStateFlags, HEX) + " [" + operatingState.deviceStateInfo + "]");
    Serial.println();
  }
  else
  {
    Serial.println("❌ Неверный формат пакета состояния работы");
    Serial.println("   Ожидалось: 4f 0A d0 06 [state] [number] [flags] [unk1] [unk2] [unk3] [crc]");
    Serial.println("   Получено: " + response);
    Serial.println("   Длина: " + String(byteCount) + " байт");
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

void WebastoSensors::getOnOffFlags()
{
  wbusQueue.add(CMD_READ_SENSOR_ON_OFF_FLAGS,
                [this](bool success, String cmd, String response)
                {
                  this->handleOnOffFlagsResponse(success, cmd, response);
                });
}

void WebastoSensors::getStatusFlags()
{
  wbusQueue.add(CMD_READ_SENSOR_STATUS_FLAGS,
                [this](bool success, String cmd, String response)
                {
                  this->handleStatusFlagsResponse(success, cmd, response);
                });
}

void WebastoSensors::getOperatingState()
{
  wbusQueue.add(CMD_READ_SENSOR_OPERATING_STATE,
                [this](bool success, String cmd, String response)
                {
                  this->handleOperatingStateResponse(success, cmd, response);
                });
}