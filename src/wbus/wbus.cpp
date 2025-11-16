#include "wbus/wbus.h"
#include "wbus/receiver/wbus-receiver.h"
#include "common/tja1020/tja1020.h"
#include "wbus/wbus-sensors.h"
#include "wbus/wbus-info.h"
#include "wbus/wbus-errors.h"

WBus wBus;

void WBus::init()
{
  initTJA1020();
  wakeUpTJA1020();
  connectionState = DISCONNECTED;
}

void WBus::wakeUp()
{
  Serial.println("🔔 Пробуждение Webasto...");

  // BREAK set - удерживаем линию в LOW 50ms
  WBusSerial.write(0x00);
  delay(50);

  // BREAK reset - отпускаем линию и ждем 50ms
  WBusSerial.flush();
  delay(50);

  Serial.println("✅ BREAK последовательность отправлена");
}

void WBus::connect()
{
  if (connectionState == CONNECTING)
  {
    Serial.println("⚠️  Подключение уже выполняется...");
    return;
  }

  connectionState = CONNECTING;
  lastConnectionAttempt = millis();

  Serial.println("🔌 Подключение к Webasto...");
  wakeUp();

  delay(100);

  webastoInfo.getMainInfo();

  wbusQueue.add(
      CMD_DIAGNOSTIC,
      [this](bool success, String cmd, String response)
      {
        if (success)
        {
          connectionState = CONNECTED;
          currentState = WBUS_STATE_READY;
          Serial.println();
          Serial.println("✅ Подключение прошло успешно");
          wbusQueue.setProcessDelay(150);

          webastoInfo.getAdditionalInfo();
          webastoSensors.getAllSensorData();
          webastoErrors.check();
        }
        else
        {
          connectionState = CONNECTION_FAILED;
          Serial.println();
          Serial.println("❌ Не удалось подключиться!");
        }
      });
}

void WBus::disconnect()
{
  wbusQueue.clear();
  connectionState = DISCONNECTED;
  currentState = WBUS_STATE_OFF;
  Serial.println();
  Serial.println("🔌 Отключение от Webasto");
}

void WBus::reconnect()
{
  if (connectionState == CONNECTED)
  {
    Serial.println();
    Serial.println("⚠️  Уже подключено");
    return;
  }

  disconnect();
  delay(1000);
  connect();
}

void WBus::updateConnectionState()
{
  // Автоматическое переподключение
  if (autoReconnect && connectionState == CONNECTION_FAILED &&
      millis() - lastConnectionAttempt > 30000)
  { // 30 секунд
    Serial.println();
    Serial.println("🔄 Попытка автоматического переподключения...");
    reconnect();
  }
}

// =============================================================================
// КОМАНДЫ УПРАВЛЕНИЯ
// =============================================================================

void WBus::startParkingHeat(int minutes)
{
  if (!isConnected())
  {
    Serial.println();
    Serial.println("❌ Нет подключения к Webasto");
    return;
  }

  minutes = constrain(minutes, 1, 255);
  String command = "F4 03 21 " + String(minutes, HEX) + " " + String(0xED ^ minutes, HEX);

  wbusQueue.add(command, [this, minutes](bool success, String cmd, String response)
                {
    if (success) {
      currentState = WBUS_STATE_HEATING;
       Serial.println();
      Serial.println("🔥 Запущен паркинг-нагрев на " + String(minutes) + " минут");
    } else {
       Serial.println();
      Serial.println("❌ Ошибка запуска паркинг-нагрева");
    } });
}

void WBus::startVentilation(int minutes)
{
  if (!isConnected())
  {
    Serial.println();
    Serial.println("❌ Нет подключения к Webasto");
    return;
  }

  minutes = constrain(minutes, 1, 255);
  String command = "F4 03 22 " + String(minutes, HEX) + " " + String(0xEE ^ minutes, HEX);

  wbusQueue.add(command, [this, minutes](bool success, String cmd, String response)
                {
    if (success) {
      currentState = WBUS_STATE_VENTILATING;
       Serial.println();
      Serial.println("💨 Запущена вентиляция на " + String(minutes) + " минут");
    } else {
       Serial.println();
      Serial.println("❌ Ошибка запуска вентиляции");
    } });
}

void WBus::startSupplementalHeat(int minutes)
{
  if (!isConnected())
  {
    Serial.println();
    Serial.println("❌ Нет подключения к Webasto");
    return;
  }

  minutes = constrain(minutes, 1, 255);
  String command = "F4 03 23 " + String(minutes, HEX) + " " + String(0xEF ^ minutes, HEX);

  wbusQueue.add(command, [this, minutes](bool success, String cmd, String response)
                {
    if (success) {
      currentState = WBUS_STATE_HEATING;
       Serial.println();
      Serial.println("🔥 Запущен дополнительный нагрев на " + String(minutes) + " минут");
    } else {
       Serial.println();
      Serial.println("❌ Ошибка запуска дополнительного нагрева");
    } });
}

void WBus::startBoostMode(int minutes)
{
  if (!isConnected())
  {
    Serial.println();
    Serial.println("❌ Нет подключения к Webasto");
    return;
  }

  minutes = constrain(minutes, 1, 255);
  String command = "F4 03 25 " + String(minutes, HEX) + " " + String(0xF1 ^ minutes, HEX);

  wbusQueue.add(command, [this, minutes](bool success, String cmd, String response)
                {
    if (success) {
      currentState = WBUS_STATE_HEATING;
       Serial.println();
      Serial.println("⚡ Запущен Boost режим на " + String(minutes) + " минут");
    } else {
       Serial.println();
      Serial.println("❌ Ошибка запуска Boost режима");
    } });
}

void WBus::shutdown()
{
  if (!isConnected())
  {
    Serial.println();
    Serial.println("❌ Нет подключения к Webasto");
    return;
  }

  wbusQueue.add(CMD_SHUTDOWN, [this](bool success, String cmd, String response)
                {
    if (success) {
      currentState = WBUS_STATE_READY;
       Serial.println();
      Serial.println("🛑 Нагреватель выключен");
    } else {
       Serial.println();
      Serial.println("❌ Ошибка выключения нагревателя");
    } });
}

void WBus::controlCirculationPump(bool enable)
{
  if (!isConnected())
  {
    Serial.println();
    Serial.println("❌ Нет подключения к Webasto");
    return;
  }

  String command = enable ? "F4 03 24 01 F3" : "F4 03 24 00 F2";

  wbusQueue.add(command, [this, enable](bool success, String cmd, String response)
                {
    if (success) {
       Serial.println();
      Serial.println(enable ? "🔛 Циркуляционный насос включен" : "🔴 Циркуляционный насос выключен");
    } else {
       Serial.println();
      Serial.println("❌ Ошибка управления циркуляционным насосом");
    } });
}

// =============================================================================
// ТЕСТИРОВАНИЕ КОМПОНЕНТОВ
// =============================================================================

void WBus::testCombustionFan(int seconds, int powerPercent)
{
  if (!isConnected())
  {
    Serial.println("❌ Нет подключения к Webasto");
    return;
  }

  seconds = constrain(seconds, 1, 255);
  powerPercent = constrain(powerPercent, 0, 100);

  String command = createTestCAFCommand(seconds, powerPercent);

  wbusQueue.add(command, [this, seconds, powerPercent](bool success, String cmd, String response)
                {
        if (success) {
            Serial.println("🌀 Тест вентилятора горения: " + String(seconds) + "сек, " + String(powerPercent) + "%");
        } else {
            Serial.println("❌ Ошибка теста вентилятора горения");
        } });
}

void WBus::testFuelPump(int seconds, int frequencyHz)
{
  if (!isConnected())
  {
    Serial.println("❌ Нет подключения к Webasto");
    return;
  }

  seconds = constrain(seconds, 1, 255);
  frequencyHz = constrain(frequencyHz, 1, 50);

  String command = createTestFuelPumpCommand(seconds, frequencyHz);

  wbusQueue.add(command, [this, seconds, frequencyHz](bool success, String cmd, String response)
                {
        if (success) {
            Serial.println("⛽ Тест топливного насоса: " + String(seconds) + "сек, " + String(frequencyHz) + "Гц");
        } else {
            Serial.println("❌ Ошибка теста топливного насоса");
        } });
}

void WBus::testGlowPlug(int seconds, int powerPercent)
{
  if (!isConnected())
  {
    Serial.println("❌ Нет подключения к Webasto");
    return;
  }

  seconds = constrain(seconds, 1, 255);
  powerPercent = constrain(powerPercent, 0, 100);

  String command = createTestGlowPlugCommand(seconds, powerPercent);

  wbusQueue.add(command, [this, seconds, powerPercent](bool success, String cmd, String response)
                {
        if (success) {
            Serial.println("🔌 Тест свечи накаливания: " + String(seconds) + "сек, " + String(powerPercent) + "%");
        } else {
            Serial.println("❌ Ошибка теста свечи накаливания");
        } });
}

void WBus::testCirculationPump(int seconds, int powerPercent)
{
  if (!isConnected())
  {
    Serial.println("❌ Нет подключения к Webasto");
    return;
  }

  seconds = constrain(seconds, 1, 255);
  powerPercent = constrain(powerPercent, 0, 100);

  String command = createTestCircPumpCommand(seconds, powerPercent);

  wbusQueue.add(command, [this, seconds, powerPercent](bool success, String cmd, String response)
                {
        if (success) {
            Serial.println("💧 Тест циркуляционного насоса: " + String(seconds) + "сек, " + String(powerPercent) + "%");
        } else {
            Serial.println("❌ Ошибка теста циркуляционного насоса");
        } });
}

void WBus::testVehicleFan(int seconds)
{
  if (!isConnected())
  {
    Serial.println("❌ Нет подключения к Webasto");
    return;
  }

  seconds = constrain(seconds, 1, 255);

  String command = createTestVehicleFanCommand(seconds);

  wbusQueue.add(command, [this, seconds](bool success, String cmd, String response)
                {
        if (success) {
            Serial.println("🌀 Тест реле вентилятора автомобиля: " + String(seconds) + "сек");
        } else {
            Serial.println("❌ Ошибка теста реле вентилятора автомобиля");
        } });
}

void WBus::testSolenoidValve(int seconds)
{
  if (!isConnected())
  {
    Serial.println("❌ Нет подключения к Webasto");
    return;
  }

  seconds = constrain(seconds, 1, 255);

  String command = createTestSolenoidCommand(seconds);

  wbusQueue.add(command, [this, seconds](bool success, String cmd, String response)
                {
        if (success) {
            Serial.println("🔘 Тест соленоидного клапана: " + String(seconds) + "сек");
        } else {
            Serial.println("❌ Ошибка теста соленоидного клапана");
        } });
}

void WBus::testFuelPreheating(int seconds, int powerPercent)
{
  if (!isConnected())
  {
    Serial.println("❌ Нет подключения к Webasto");
    return;
  }

  seconds = constrain(seconds, 1, 255);
  powerPercent = constrain(powerPercent, 0, 100);

  String command = createTestFuelPreheatCommand(seconds, powerPercent);

  wbusQueue.add(command, [this, seconds, powerPercent](bool success, String cmd, String response)
                {
        if (success) {
            Serial.println("🔥 Тест подогрева топлива: " + String(seconds) + "сек, " + String(powerPercent) + "%");
        } else {
            Serial.println("❌ Ошибка теста подогрева топлива");
        } });
}

// =============================================================================
// ВЫВОД СТАТУСА
// =============================================================================

void WBus::printStatus()
{
  Serial.println();
  Serial.println("═══════════════════════════════════════════════════════════");
  Serial.println("                 📊 СТАТУС WEBASTO                        ");
  Serial.println("═══════════════════════════════════════════════════════════");

  // Статус подключения
  String connStatus;
  switch (connectionState)
  {
  case DISCONNECTED:
    connStatus = "🔴 Отключено";
    break;
  case CONNECTING:
    connStatus = "🟡 Подключение...";
    break;
  case CONNECTED:
    connStatus = "🟢 Подключено";
    break;
  case CONNECTION_FAILED:
    connStatus = "🔴 Ошибка подключения";
    break;
  }
  Serial.println("Подключение:        " + connStatus);

  // Состояние нагревателя
  String state;
  switch (currentState)
  {
  case WBUS_STATE_OFF:
    state = "🔴 Выключен";
    break;
  case WBUS_STATE_INITIALIZING:
    state = "🟡 Инициализация";
    break;
  case WBUS_STATE_READY:
    state = "🟢 Готов";
    break;
  case WBUS_STATE_HEATING:
    state = "🔥 Нагрев";
    break;
  case WBUS_STATE_VENTILATING:
    state = "💨 Вентиляция";
    break;
  case WBUS_STATE_ERROR:
    state = "🚨 Ошибка";
    break;
  }
  Serial.println();
  Serial.println("Состояние:          " + state);

  // Дополнительная информация
  Serial.println("Автопереподключение:" + String(autoReconnect ? "✅ Вкл" : "❌ Выкл"));
  Serial.println("═══════════════════════════════════════════════════════════");
  Serial.println();
}

// =============================================================================
// ОБРАБОТКА КОМАНД ПОЛЬЗОВАТЕЛЯ
// =============================================================================

void WBus::processQueue()
{
  // Обновляем статус подключения
  updateConnectionState();

  // Проверяем команды от пользователя
  if (Serial.available())
  {
    String command = Serial.readString();
    command.trim();
    command.toLowerCase();

    if (command == "wake" || command == "w")
    {
      wakeUpTJA1020();
    }
    else if (command == "sleep" || command == "s")
    {
      sleepTJA1020();
    }
    else if (command == "connect" || command == "con")
    {
      connect();
    }
    else if (command == "disconnect" || command == "dc")
    {
      disconnect();
    }
    else if (command == "reconnect" || command == "rc")
    {
      reconnect();
    }
    else if (command == "status" || command == "st")
    {
      printStatus();
    }
    else if (command == "info" || command == "i")
    {
      webastoInfo.printInfo();
    }
    else if (command == "sensors")
    {
      webastoSensors.printSensorData();
    }
    else if (command == "errors" || command == "err")
    {
      webastoErrors.check();
    }
    else if (command == "clear" || command == "clr")
    {
      webastoErrors.clear();
    }
    // Команды управления
    else if (command == "heat" || command == "h")
    {
      startParkingHeat(30);
    }
    else if (command == "vent" || command == "v")
    {
      startVentilation(30);
    }
    else if (command == "boost" || command == "b")
    {
      startBoostMode(15);
    }
    else if (command == "stop" || command == "off")
    {
      shutdown();
    }
    else if (command == "pump on")
    {
      controlCirculationPump(true);
    }
    else if (command == "pump off")
    {
      controlCirculationPump(false);
    }
    // Тестирование
    else if (command == "test fan")
    {
      testCombustionFan(5, 50);
    }
    else if (command == "test fuel")
    {
      testFuelPump(3, 10);
    }
    else if (command == "test glow")
    {
      testGlowPlug(5, 50);
    }
    else if (command == "test circ")
    {
      testCirculationPump(5, 80);
    }
    else if (command == "test vehicle")
    {
      testVehicleFan(5);
    }
    else if (command == "test solenoid")
    {
      testSolenoidValve(5);
    }
    else if (command == "test preheat")
    {
      testFuelPreheating(5, 50);
    }
    else if (command == "help" || command == "h")
    {
      printHelp();
    }
    else if (command == "break")
    {
      wakeUp();
    }
    else
    {
      wbusQueue.add(command);
    }
  }

  wbusQueue.process();
}

void WBus::processReceiver()
{
  wBusReceiver.process();
}