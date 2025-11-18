#include "wbus/wbus.h"
#include "kline-receiver/kline-receiver.h"
#include "common/tja1020/tja1020.h"
#include "common/timeout/timeout.h"
#include "wbus/wbus-sensors.h"
#include "wbus/wbus-info.h"
#include "wbus/wbus-errors.h"
#include "server/socket-server.h"
#include "server/api-server.h"

WBus wBus;

Timeout keepAliveTimeout(25000);

void WBus::init()
{
  initTJA1020();
  wakeUpTJA1020();
  connectionState = DISCONNECTED;
}

void WBus::wakeUp()
{
  // BREAK set - удерживаем линию в LOW 50ms
  KLineSerial.write(0x00);
  delay(50);

  // BREAK reset - отпускаем линию и ждем 50ms
  KLineSerial.flush();
  delay(50);
}

void WBus::connect()
{
  if (connectionState == CONNECTING)
  {
    Serial.println();
    Serial.println("⚠️  Подключение уже выполняется...");
    return;
  }

  connectionState = CONNECTING;
  lastConnectionAttempt = millis();

  Serial.println();
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
          Serial.println();
          Serial.println("✅ Подключение прошло успешно");
          wbusQueue.setInterval(550);

          webastoInfo.getAdditionalInfo();
          webastoSensors.getAllSensorData(true);
          webastoErrors.check(true);
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
  Serial.println();
  Serial.println("🔌 Отключение от Webasto");
}

void WBus::checkConnectionTimeout()
{
  if (connectionState == CONNECTED && _lastRxTime > 0)
  {
    unsigned long timeSinceLastRx = millis() - _lastRxTime;

    if (timeSinceLastRx > 5000)
    { // 5 секунд без ответа
      connectionState = CONNECTION_FAILED;
    }
  }
}

// =============================================================================
// КОМАНДЫ УПРАВЛЕНИЯ
// =============================================================================

void WBus::shutdown()
{
  if (!isConnected())
    wakeUp();

  wbusQueue.add(CMD_SHUTDOWN, [this](bool success, String cmd, String response)
                {
        if (success) {
            Serial.println();
            Serial.println("🛑 Нагреватель выключен");
        } else {
          Serial.println();
            Serial.println("❌ Ошибка выключения нагревателя");
        } });
}

void WBus::startParkingHeat(int minutes)
{
  if (!isConnected())
    wakeUp();

  minutes = constrain(minutes, 1, 255);
  String command = createParkHeatCommand(minutes);

  wbusQueue.add(command, [this, minutes](bool success, String cmd, String response)
                {
        if (success) {
            currentState = WBUS_STATE_PARKING_HEAT;
            Serial.println();
            Serial.println("🔥 Паркинг-нагрев запущен на " + String(minutes) + " минут");
          
        } else {
          Serial.println();
            Serial.println("❌ Ошибка запуска паркинг-нагрева");
        } });
}

void WBus::startVentilation(int minutes)
{
  if (!isConnected())
    wakeUp();

  minutes = constrain(minutes, 1, 255);
  String command = createVentilateCommand(minutes);

  wbusQueue.add(command, [this, minutes](bool success, String cmd, String response)
                {
        if (success) {
            currentState = WBUS_STATE_VENTILATION;
            Serial.println();
            Serial.println("💨 Вентиляция запущена на " + String(minutes) + " минут");
    
        } else {
          Serial.println();
            Serial.println("❌ Ошибка запуска вентиляции");
        } });
}

void WBus::startSupplementalHeat(int minutes)
{
  if (!isConnected())
    wakeUp();

  minutes = constrain(minutes, 1, 255);
  String command = createSuppHeatCommand(minutes);

  wbusQueue.add(command, [this, minutes](bool success, String cmd, String response)
                {
        if (success) {
            currentState = WBUS_STATE_SUPP_HEAT;
            Serial.println();
            Serial.println("🔥 Дополнительный нагрев запущен на " + String(minutes) + " минут");
        } else {
          Serial.println();
            Serial.println("❌ Ошибка запуска дополнительного нагрева");
        } });
}

void WBus::controlCirculationPump(bool enable)
{
  if (!isConnected())
    wakeUp();

  String command = createCircPumpCommand(enable);

  wbusQueue.add(command, [this, enable](bool success, String cmd, String response)
                {
        if (success) {
          currentState = WBUS_STATE_CIRC_PUMP;
          Serial.println();
          Serial.println(enable ? "🔛 Циркуляционный насос включен" : "🔴 Циркуляционный насос выключен");
        } else {
          Serial.println();
            Serial.println("❌ Ошибка управления циркуляционным насосом");
        } });
}

void WBus::startBoostMode(int minutes)
{
  if (!isConnected())
    wakeUp();

  minutes = constrain(minutes, 1, 255);
  String command = createBoostCommand(minutes);

  wbusQueue.add(command, [this, minutes](bool success, String cmd, String response)
                {
        if (success) {
            currentState = WBUS_STATE_BOOST;
            Serial.println();
            Serial.println("⚡ Boost режим запущен на " + String(minutes) + " минут");
        } else {
          Serial.println();
            Serial.println("❌ Ошибка запуска Boost режима");
        } });
}

// =============================================================================
// ТЕСТИРОВАНИЕ КОМПОНЕНТОВ
// =============================================================================

void WBus::testCombustionFan(int seconds, int powerPercent)
{
  if (!isConnected())
  {
    wakeUp();
  }

  seconds = constrain(seconds, 1, 255);
  powerPercent = constrain(powerPercent, 0, 100);

  String command = createTestCAFCommand(seconds, powerPercent);

  wbusQueue.add(command, [this, seconds, powerPercent](bool success, String cmd, String response)
                {
        if (success) {
          Serial.println();
            Serial.println("🌀 Тест вентилятора горения: " + String(seconds) + "сек, " + String(powerPercent) + "%");
        } else {
          Serial.println();
            Serial.println("❌ Ошибка теста вентилятора горения");
        } });
}

void WBus::testFuelPump(int seconds, int frequencyHz)
{
  if (!isConnected())
  {
    wakeUp();
  }

  seconds = constrain(seconds, 1, 255);
  frequencyHz = constrain(frequencyHz, 1, 50);

  String command = createTestFuelPumpCommand(seconds, frequencyHz);

  wbusQueue.add(command, [this, seconds, frequencyHz](bool success, String cmd, String response)
                {
        if (success) {
          Serial.println();
            Serial.println("⛽ Тест топливного насоса: " + String(seconds) + "сек, " + String(frequencyHz) + "Гц");
        } else {
          Serial.println();
            Serial.println("❌ Ошибка теста топливного насоса");
        } });
}

void WBus::testGlowPlug(int seconds, int powerPercent)
{
  if (!isConnected())
  {
    wakeUp();
  }

  seconds = constrain(seconds, 1, 255);
  powerPercent = constrain(powerPercent, 0, 100);

  String command = createTestGlowPlugCommand(seconds, powerPercent);

  wbusQueue.add(command, [this, seconds, powerPercent](bool success, String cmd, String response)
                {
        if (success) {
          Serial.println();
            Serial.println("🔌 Тест свечи накаливания: " + String(seconds) + "сек, " + String(powerPercent) + "%");
        } else {
          Serial.println();
            Serial.println("❌ Ошибка теста свечи накаливания");
        } });
}

void WBus::testCirculationPump(int seconds, int powerPercent)
{
  if (!isConnected())
  {
    wakeUp();
  }

  seconds = constrain(seconds, 1, 255);
  powerPercent = constrain(powerPercent, 0, 100);

  String command = createTestCircPumpCommand(seconds, powerPercent);

  wbusQueue.add(command, [this, seconds, powerPercent](bool success, String cmd, String response)
                {
        if (success) {
          Serial.println();
            Serial.println("💧 Тест циркуляционного насоса: " + String(seconds) + "сек, " + String(powerPercent) + "%");
        } else {
          Serial.println();
            Serial.println("❌ Ошибка теста циркуляционного насоса");
        } });
}

void WBus::testVehicleFan(int seconds)
{
  if (!isConnected())
  {
    wakeUp();
  }

  seconds = constrain(seconds, 1, 255);

  String command = createTestVehicleFanCommand(seconds);

  wbusQueue.add(command, [this, seconds](bool success, String cmd, String response)
                {
        if (success) {
          Serial.println();
            Serial.println("🌀 Тест реле вентилятора автомобиля: " + String(seconds) + "сек");
        } else {
          Serial.println();
            Serial.println("❌ Ошибка теста реле вентилятора автомобиля");
        } });
}

void WBus::testSolenoidValve(int seconds)
{
  if (!isConnected())
  {
    wakeUp();
  }

  seconds = constrain(seconds, 1, 255);

  String command = createTestSolenoidCommand(seconds);

  wbusQueue.add(command, [this, seconds](bool success, String cmd, String response)
                {
        if (success) {
          Serial.println();
            Serial.println("🔘 Тест соленоидного клапана: " + String(seconds) + "сек");
        } else {
          Serial.println();
            Serial.println("❌ Ошибка теста соленоидного клапана");
        } });
}

void WBus::testFuelPreheating(int seconds, int powerPercent)
{
  if (!isConnected())
  {
    wakeUp();
  }

  seconds = constrain(seconds, 1, 255);
  powerPercent = constrain(powerPercent, 0, 100);

  String command = createTestFuelPreheatCommand(seconds, powerPercent);

  wbusQueue.add(command, [this, seconds, powerPercent](bool success, String cmd, String response)
                {
        if (success) {
          Serial.println();
            Serial.println("🔥 Тест подогрева топлива: " + String(seconds) + "сек, " + String(powerPercent) + "%");
        } else {
          Serial.println();
            Serial.println("❌ Ошибка теста подогрева топлива");
        } });
}

String WBus::getKeepAliveCommandForCurrentState()
{
  switch (currentState)
  {
  case WBUS_STATE_PARKING_HEAT:
    return CMD_KEEPALIVE_PARKING;
  case WBUS_STATE_VENTILATION:
    return CMD_KEEPALIVE_VENT;
  case WBUS_STATE_SUPP_HEAT:
    return CMD_KEEPALIVE_SUPP_HEAT;
  case WBUS_STATE_BOOST:
    return CMD_KEEPALIVE_BOOST;
  case WBUS_STATE_CIRC_PUMP:
    return CMD_KEEPALIVE_CIRC_PUMP;
  case WBUS_STATE_OFF:
  case WBUS_STATE_READY:
  case WBUS_STATE_STARTUP:
  case WBUS_STATE_SHUTDOWN:
  case WBUS_STATE_ERROR:
  default:
    return ""; // Эти состояния не требуют keep-alive
  }
}

String WBus::getStateName()
{
  switch (currentState)
  {
  case WBUS_STATE_OFF:
    return "🔴 Выключен";
  case WBUS_STATE_READY:
    return "🟢 Готов";
  case WBUS_STATE_PARKING_HEAT:
    return "🔥 Паркинг-нагрев";
  case WBUS_STATE_VENTILATION:
    return "💨 Вентиляция";
  case WBUS_STATE_SUPP_HEAT:
    return "🔥 Доп. нагрев";
  case WBUS_STATE_BOOST:
    return "⚡ Boost";
  case WBUS_STATE_CIRC_PUMP:
    return "💧 Цирк. насос";
  default:
    return "❓ Неизвестно";
  }
}

void WBus::updateStateFromSensors()
{
  // Используем данные из webastoSensors которые уже обновляются циклически
  webastoSensors.getStatusFlags();
  webastoSensors.getOnOffFlags();
  StatusFlags flags = webastoSensors.getStatusFlagsData();
  OnOffFlags onOff = webastoSensors.getOnOffFlagsData();

  WebastoState newState = determineStateFromFlags(flags, onOff);

  if (newState != currentState)
  {
    currentState = newState;
  }
}

WebastoState WBus::determineStateFromFlags(const StatusFlags &flags, OnOffFlags &onOff)
{
  // Анализируем флаги статуса
  if (flags.parkingHeatRequest)
    return WBUS_STATE_PARKING_HEAT;
  if (flags.ventilationRequest)
    return WBUS_STATE_VENTILATION;
  if (flags.supplementalHeatRequest)
    return WBUS_STATE_SUPP_HEAT;
  if (flags.boostMode)
    return WBUS_STATE_BOOST;

  // Проверяем активные компоненты
  if (onOff.circulationPump && !onOff.combustionAirFan && !onOff.fuelPump)
    return WBUS_STATE_CIRC_PUMP;

  // Если ничего активного не найдено, но главный выключатель включен
  if (flags.mainSwitch)
    return WBUS_STATE_READY;

  return WBUS_STATE_OFF;
}

void WBus::processCommands()
{
  // Проверяем команды от пользователя
  if (Serial.available())
  {
    String command = Serial.readString();
    command.trim();
    command.toLowerCase();

    if (command == "start")
    {
      startParkingHeat();
    }
    else if (command == "stop")
    {
      shutdown();
    }
    else if (command == "wake" || command == "w")
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
    else if (command == "help" || command == "h")
    {
      printHelp();
    }
    else
    {
      if (!isConnected())
        wakeUp();

      wbusQueue.addPriority(command);
    }
  }
}

void WBus::processKeepAlive()
{
  if (keepAliveTimeout.isReady() && !getKeepAliveCommandForCurrentState().isEmpty())
  {
    Serial.println();
    Serial.print("Статус: " + getStateName());
    if (!isConnected())
    {
      wakeUp();
    }

    // Сначала обновляем состояние на основе текущих данных
    updateStateFromSensors();

    String keepAliveCommand = getKeepAliveCommandForCurrentState();

    if (!keepAliveCommand.isEmpty())
    {
      wbusQueue.addPriority(keepAliveCommand, [this](bool success, String cmd, String response)
                            {
                if (!success) {
                    Serial.println("❌ Keep-alive не доставлен для состояния: " + getStateName());
                } });
    }
  }
}

void WBus::process()
{
  kLineReceiver.process();
  processCommands();
  processKeepAlive();

  wbusQueue.process();

  if (kLineReceiver.kLineReceivedData.isRxReceived())
  {
    socketServer.sendRx(kLineReceiver.kLineReceivedData.getRxData());
    _lastRxTime = millis();
  }

  if (kLineReceiver.kLineReceivedData.isTxReceived())
  {
    socketServer.sendTx(kLineReceiver.kLineReceivedData.getTxData());
  }

  socketServer.loop();
  apiServer.loop();

  // Проверяем таймаут соединения
  checkConnectionTimeout();
}
