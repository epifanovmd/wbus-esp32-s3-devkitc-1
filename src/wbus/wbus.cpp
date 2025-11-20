#include "wbus/wbus.h"
#include "kline-receiver/kline-receiver.h"
#include "common/tja1020/tja1020.h"
#include "common/timeout/timeout.h"
#include "wbus/wbus-sensors.h"
#include "wbus/wbus-info.h"
#include "wbus/wbus-queue.h"
#include "wbus/wbus-errors.h"
#include "wbus.constants.h"
#include "server/socket-server.h"
#include "server/api-server.h"

WBus wBus;

Timeout keepAliveTimeout(15000);

String WebastoStateNames[] = {
    "WBUS_STATE_OFF",          // Выключен
    "WBUS_STATE_READY",        // Готов к работе
    "WBUS_STATE_PARKING_HEAT", // Паркинг-нагрев
    "WBUS_STATE_VENTILATION",  // Вентиляция
    "WBUS_STATE_SUPP_HEAT",    // Дополнительный нагрев
    "WBUS_STATE_BOOST",        // Boost режим
    "WBUS_STATE_CIRC_PUMP",    // Только циркуляционный насос
    "WBUS_STATE_STARTUP",      // Запуск
    "WBUS_STATE_SHUTDOWN",     // Выключение
    "WBUS_STATE_ERROR"         // Ошибка
};

String ConnectionStateNames[] = {
    "DISCONNECTED",
    "CONNECTING",
    "CONNECTED",
    "CONNECTION_FAILED"};

void WBus::setConnectionState(ConnectionState newState)
{
  if (connectionState == newState)
    return;

  ConnectionState oldState = connectionState;
  connectionState = newState;

  socketServer.sendSystemStatus("connection_state", ConnectionStateNames[newState], ConnectionStateNames[oldState]);
  Serial.println();
  Serial.print(ConnectionStateNames[oldState] + " –> " + ConnectionStateNames[newState]);

  // Дополнительные действия при изменении состояния
  switch (newState)
  {
  case CONNECTING:
    neopixelWrite(RGB_PIN, 255 / 4, 165 / 4, 0);
    break;
  case CONNECTION_FAILED:
    neopixelWrite(RGB_PIN, 255 / 4, 0, 0);
    break;
  case CONNECTED:
    neopixelWrite(RGB_PIN, 0, 255 / 4, 0);
    break;
  case DISCONNECTED:
    neopixelWrite(RGB_PIN, 0, 0, 0);
    webastoInfo.clear();
    webastoSensors.clear();
    webastoErrors.clear();
    _logging = false;
    break;
  }
}

void WBus::setState(WebastoState newState)
{
  if (currentState == newState)
    return;

  WebastoState oldState = currentState;
  currentState = newState;

  socketServer.sendSystemStatus("state", WebastoStateNames[newState], WebastoStateNames[oldState]);
  Serial.println();
  Serial.print(WebastoStateNames[oldState] + " –> " + WebastoStateNames[newState]);
}

void WBus::init()
{
  initTJA1020();
  wakeUpTJA1020();
  neopixelWrite(RGB_PIN, 0, 0, 0);
  setConnectionState(DISCONNECTED);
  setState(WBUS_STATE_OFF);
  _logging = false;
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

void WBus::connect(std::function<void(String tx, String rx)> callback)
{
  if (connectionState == CONNECTING)
  {
    Serial.println();
    Serial.println("⚠️  Подключение уже выполняется...");
    if (callback)
      callback("", "");
    return;
  }

  setConnectionState(CONNECTING);

  wakeUp();
  delay(100);

  // Получаем основную информацию об устройстве
  webastoInfo.getWBusVersion();
  webastoInfo.getDeviceName();
  webastoInfo.getWBusCode();

  // Запускаем диагностику
  wbusQueue.add(
      CMD_DIAGNOSTIC,
      [this, callback](String tx, String rx)
      {
        if (!rx.isEmpty())
        {
          // Получаем дополнительную информацию
          webastoInfo.getDeviceID();
          webastoInfo.getControllerManufactureDate();
          webastoInfo.getHeaterManufactureDate();
          webastoInfo.getCustomerID();
          webastoInfo.getSerialNumber([](String tx, String rx, DecodedTextData *serial)
                                      {
          DynamicJsonDocument doc(512);

          doc["wbus_version"] = webastoInfo.getWBusVersionData();
          doc["device_name"] = webastoInfo.getDeviceNameData();;
          doc["device_id"] = webastoInfo.getDeviceIDData();
          doc["serial_number"] = webastoInfo.getSerialNumberData();
          doc["controller_manufacture_date"] = webastoInfo.getControllerManufactureDateData();
          doc["heater_manufacture_date"] = webastoInfo.getHeaterManufactureDateData();
          doc["customer_id"] = webastoInfo.getCustomerIDData();;
          doc["wbus_code"] = webastoInfo.getWBusCodeData();;
          doc["supported_functions"] = webastoInfo.getSupportedFunctionsData();

          String json;
          serializeJson(doc, json);
          socketServer.send("device_info", json); });

          wbusQueue.setInterval(200);

          // Запускаем периодический опрос сенсоров
          webastoSensors.getOperationalInfo(true, [this](String tx, String rx, OperationalMeasurements *measurements)
                                            {
          if (measurements != nullptr) {
            String json = webastoSensors.createJsonOperationalInfo( * measurements);
            socketServer.send("operational_measurements", json);
          } });

          webastoSensors.getFuelSettings(false, [this](String tx, String rx, FuelSettings *fuel)
                                         {
          if (fuel != nullptr) {
            String json = webastoSensors.createJsonFuelSettings( * fuel);
            socketServer.send("fuel_settings", json);
          } });

          webastoSensors.getOnOffFlags(true, [this](String tx, String rx, OnOffFlags *onOff)
                                       {
          if (onOff != nullptr) {
            String json = webastoSensors.createJsonOnOffFlags( * onOff);
            socketServer.send("on_off_flags", json);
          } });

          webastoSensors.getStatusFlags(true, [this](String tx, String rx, StatusFlags *status)
                                        {
          if (status != nullptr) {
            String json = webastoSensors.createJsonStatusFlags( * status);
            socketServer.send("status_flags", json);
          } });

          webastoSensors.getOperatingState(true, [this](String tx, String rx, OperatingState *state)
                                           {
          if (state != nullptr) {
            String json = webastoSensors.createJsonOperatingState( * state);
            socketServer.send("operating_state", json);
          } });

          webastoSensors.getSubsystemsStatus(true, [this](String tx, String rx, SubsystemsStatus *subsystems)
                                             {
          if (subsystems != nullptr) {
            String json = webastoSensors.createJsonSubsystemsStatus( * subsystems);
            socketServer.send("subsystems_status", json);
          } });

          webastoErrors.check(true, [this](String tx, String rx, ErrorCollection *errors)
                              {
          if (errors != nullptr) {
            String json = webastoErrors.createJsonErrors( * errors);
            socketServer.send("errors", json);
          } });

          Serial.println();
          Serial.println("✅ Подключение к Webasto установлено");
          setConnectionState(CONNECTED);
        }
        else
        {
          Serial.println();
          Serial.println("❌ Ошибка подключения к Webasto");
          setConnectionState(CONNECTION_FAILED);
        }

        if (callback)
          callback(tx, rx);
      });
}

void WBus::disconnect()
{
  wbusQueue.clear();
  setConnectionState(DISCONNECTED);
  // Serial.println();
  // Serial.println("🔌 Отключение от Webasto");
}

// =============================================================================
// КОМАНДЫ УПРАВЛЕНИЯ С КОЛБЭКАМИ
// =============================================================================

void WBus::shutdown(std::function<void(String tx, String rx)> callback)
{
  if (!isConnected())
    wakeUp();

  wbusQueue.add(CMD_SHUTDOWN, [this, callback](String tx, String rx)
                {
    if (!rx.isEmpty()) {
      Serial.println();
      Serial.println("🛑 Нагреватель выключен");
    } else {
      Serial.println();
      Serial.println("❌ Ошибка выключения нагревателя");
    }
    if (callback) callback(tx, rx); });
}

void WBus::startParkingHeat(int minutes, std::function<void(String tx, String rx)> callback)
{
  if (!isConnected())
    wakeUp();

  minutes = constrain(minutes, 1, 255);
  String command = createParkHeatCommand(minutes);

  wbusQueue.add(command, [this, minutes, callback](String tx, String rx)
                {
    if (!rx.isEmpty()) {
      setState(WBUS_STATE_PARKING_HEAT);
      Serial.println();
      Serial.println("🔥 Паркинг-нагрев запущен на " + String(minutes) + " минут");
    } else {
      Serial.println();
      Serial.println("❌ Ошибка запуска паркинг-нагрева");
    }
    if (callback) callback(tx, rx); });
}

void WBus::startVentilation(int minutes, std::function<void(String tx, String rx)> callback)
{
  if (!isConnected())
    wakeUp();

  minutes = constrain(minutes, 1, 255);
  String command = createVentilateCommand(minutes);

  wbusQueue.add(command, [this, minutes, callback](String tx, String rx)
                {
    if (!rx.isEmpty()) {
      setState(WBUS_STATE_VENTILATION);
      Serial.println();
      Serial.println("💨 Вентиляция запущена на " + String(minutes) + " минут");
    } else {
      Serial.println();
      Serial.println("❌ Ошибка запуска вентиляции");
    }
    if (callback) callback(tx, rx); });
}

void WBus::startSupplementalHeat(int minutes, std::function<void(String tx, String rx)> callback)
{
  if (!isConnected())
    wakeUp();

  minutes = constrain(minutes, 1, 255);
  String command = createSuppHeatCommand(minutes);

  wbusQueue.add(command, [this, minutes, callback](String tx, String rx)
                {
    if (!rx.isEmpty()) {
      setState(WBUS_STATE_SUPP_HEAT);
      Serial.println();
      Serial.println("🔥 Дополнительный нагрев запущен на " + String(minutes) + " минут");
    } else {
      Serial.println();
      Serial.println("❌ Ошибка запуска дополнительного нагрева");
    }
    if (callback) callback(tx, rx); });
}

void WBus::controlCirculationPump(bool enable, std::function<void(String tx, String rx)> callback)
{
  if (!isConnected())
    wakeUp();

  String command = createCircPumpCommand(enable);

  wbusQueue.add(command, [this, enable, callback](String tx, String rx)
                {
    if (!rx.isEmpty()) {
      setState(WBUS_STATE_CIRC_PUMP);
      Serial.println();
      Serial.println(enable ? "🔛 Циркуляционный насос включен" : "🔴 Циркуляционный насос выключен");
    } else {
      Serial.println();
      Serial.println("❌ Ошибка управления циркуляционным насосом");
    }
    if (callback) callback(tx, rx); });
}

void WBus::startBoostMode(int minutes, std::function<void(String tx, String rx)> callback)
{
  if (!isConnected())
    wakeUp();

  minutes = constrain(minutes, 1, 255);
  String command = createBoostCommand(minutes);

  wbusQueue.add(command, [this, minutes, callback](String tx, String rx)
                {
    if (!rx.isEmpty()) {
      setState(WBUS_STATE_BOOST);
      Serial.println();
      Serial.println("⚡ Boost режим запущен на " + String(minutes) + " минут");
    } else {
      Serial.println();
      Serial.println("❌ Ошибка запуска Boost режима");
    }
    if (callback) callback(tx, rx); });
}

// =============================================================================
// ТЕСТИРОВАНИЕ КОМПОНЕНТОВ С КОЛБЭКАМИ
// =============================================================================

void WBus::testCombustionFan(int seconds, int powerPercent, std::function<void(String tx, String rx)> callback)
{
  if (!isConnected())
    wakeUp();

  seconds = constrain(seconds, 1, 255);
  powerPercent = constrain(powerPercent, 0, 100);

  String command = createTestCAFCommand(seconds, powerPercent);

  wbusQueue.add(command, [this, seconds, powerPercent, callback](String tx, String rx)
                {
    if (!rx.isEmpty()) {
      Serial.println();
      Serial.println("🌀 Тест вентилятора горения: " + String(seconds) + "сек, " + String(powerPercent) + "%");
    } else {
      Serial.println();
      Serial.println("❌ Ошибка теста вентилятора горения");
    }
    if (callback) callback(tx, rx); });
}

void WBus::testFuelPump(int seconds, int frequencyHz, std::function<void(String tx, String rx)> callback)
{
  if (!isConnected())
    wakeUp();

  seconds = constrain(seconds, 1, 255);
  frequencyHz = constrain(frequencyHz, 1, 50);

  String command = createTestFuelPumpCommand(seconds, frequencyHz);

  wbusQueue.add(command, [this, seconds, frequencyHz, callback](String tx, String rx)
                {
    if (!rx.isEmpty()) {
      Serial.println();
      Serial.println("⛽ Тест топливного насоса: " + String(seconds) + "сек, " + String(frequencyHz) + "Гц");
    } else {
      Serial.println();
      Serial.println("❌ Ошибка теста топливного насоса");
    }
    if (callback) callback(tx, rx); });
}

void WBus::testGlowPlug(int seconds, int powerPercent, std::function<void(String tx, String rx)> callback)
{
  if (!isConnected())
    wakeUp();

  seconds = constrain(seconds, 1, 255);
  powerPercent = constrain(powerPercent, 0, 100);

  String command = createTestGlowPlugCommand(seconds, powerPercent);

  wbusQueue.add(command, [this, seconds, powerPercent, callback](String tx, String rx)
                {
    if (!rx.isEmpty()) {
      Serial.println();
      Serial.println("🔌 Тест свечи накаливания: " + String(seconds) + "сек, " + String(powerPercent) + "%");
    } else {
      Serial.println();
      Serial.println("❌ Ошибка теста свечи накаливания");
    }
    if (callback) callback(tx, rx); });
}

void WBus::testCirculationPump(int seconds, int powerPercent, std::function<void(String tx, String rx)> callback)
{
  if (!isConnected())
    wakeUp();

  seconds = constrain(seconds, 1, 255);
  powerPercent = constrain(powerPercent, 0, 100);

  String command = createTestCircPumpCommand(seconds, powerPercent);

  wbusQueue.add(command, [this, seconds, powerPercent, callback](String tx, String rx)
                {
    if (!rx.isEmpty()) {
      Serial.println();
      Serial.println("💧 Тест циркуляционного насоса: " + String(seconds) + "сек, " + String(powerPercent) + "%");
    } else {
      Serial.println();
      Serial.println("❌ Ошибка теста циркуляционного насоса");
    }
    if (callback) callback(tx, rx); });
}

void WBus::testVehicleFan(int seconds, std::function<void(String tx, String rx)> callback)
{
  if (!isConnected())
    wakeUp();

  seconds = constrain(seconds, 1, 255);

  String command = createTestVehicleFanCommand(seconds);

  wbusQueue.add(command, [this, seconds, callback](String tx, String rx)
                {
    if (!rx.isEmpty()) {
      Serial.println();
      Serial.println("🌀 Тест реле вентилятора автомобиля: " + String(seconds) + "сек");
    } else {
      Serial.println();
      Serial.println("❌ Ошибка теста реле вентилятора автомобиля");
    }
    if (callback) callback(tx, rx); });
}

void WBus::testSolenoidValve(int seconds, std::function<void(String tx, String rx)> callback)
{
  if (!isConnected())
    wakeUp();

  seconds = constrain(seconds, 1, 255);

  String command = createTestSolenoidCommand(seconds);

  wbusQueue.add(command, [this, seconds, callback](String tx, String rx)
                {
    if (!rx.isEmpty()) {
      Serial.println();
      Serial.println("🔘 Тест соленоидного клапана: " + String(seconds) + "сек");
    } else {
      Serial.println();
      Serial.println("❌ Ошибка теста соленоидного клапана");
    }
    if (callback) callback(tx, rx); });
}

void WBus::testFuelPreheating(int seconds, int powerPercent, std::function<void(String tx, String rx)> callback)
{
  if (!isConnected())
    wakeUp();

  seconds = constrain(seconds, 1, 255);
  powerPercent = constrain(powerPercent, 0, 100);

  String command = createTestFuelPreheatCommand(seconds, powerPercent);

  wbusQueue.add(command, [this, seconds, powerPercent, callback](String tx, String rx)
                {
    if (!rx.isEmpty()) {
      Serial.println();
      Serial.println("🔥 Тест подогрева топлива: " + String(seconds) + "сек, " + String(powerPercent) + "%");
    } else {
      Serial.println();
      Serial.println("❌ Ошибка теста подогрева топлива");
    }
    if (callback) callback(tx, rx); });
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

String WBus::getStateName(WebastoState state)
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

String WBus::getCurrentStateName()
{
  return getStateName(currentState);
}

void WBus::updateStateFromSensors(std::function<void()> callback)
{
  webastoSensors.getStatusFlags();
  webastoSensors.getOnOffFlags(false, [this, callback](String tx, String rx, OnOffFlags *onOff)
                               {
    StatusFlags * flags = webastoSensors.getStatusFlagsData();

    WebastoState newState = determineStateFromFlags(flags, onOff);

    if (newState != currentState) {
      setState(newState);
    }

    if (callback != nullptr) {
      callback();
    } });
}

WebastoState WBus::determineStateFromFlags(const StatusFlags *flags, OnOffFlags *onOff)
{
  // Анализируем флаги статуса
  if (flags->parkingHeatRequest)
    return WBUS_STATE_PARKING_HEAT;
  if (flags->ventilationRequest)
    return WBUS_STATE_VENTILATION;
  if (flags->supplementalHeatRequest)
    return WBUS_STATE_SUPP_HEAT;
  if (flags->boostMode)
    return WBUS_STATE_BOOST;

  // Проверяем активные компоненты
  if (onOff->circulationPump && !onOff->combustionAirFan && !onOff->fuelPump)
    return WBUS_STATE_CIRC_PUMP;

  // Если ничего активного не найдено, но главный выключатель включен
  if (flags->mainSwitch)
    return WBUS_STATE_READY;

  return WBUS_STATE_OFF;
}

void WBus::processSerialCommands()
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
      webastoErrors.printErrors();
    }
    else if (command == "clear" || command == "clr")
    {
      webastoErrors.reset();
    }
    else if (command == "log")
    {
      if (wBus.isLogging())
      {
        wBus.stopLogging();
      }
      else
      {
        wBus.startLogging();
      }
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
    Serial.print("Статус: " + getStateName(currentState));
    if (!isConnected())
    {
      wakeUp();
    }

    // Сначала обновляем состояние на основе текущих данных состояния Webasto
    updateStateFromSensors(
        [this]()
        {
          String keepAliveCommand = getKeepAliveCommandForCurrentState();

          if (!keepAliveCommand.isEmpty())
          {
            wbusQueue.addPriority(keepAliveCommand, [this](String tx, String rx)
                                  {
            if (rx.isEmpty()) {
              Serial.println("❌ Keep-alive не доставлен для состояния: " + getStateName(currentState));
            } });
          }
        });
  }
}

void WBus::checkConnection()
{
  if (wbusQueue.isEmpty() && connectionState != DISCONNECTED)
  {
    setConnectionState(DISCONNECTED);
  }
  else if (connectionState == CONNECTION_FAILED)
  {
    // Если было состояние ошибки, но мы получили успешный ответ - восстанавливаем соединение
    if (_lastRxTime > 0 && millis() - _lastRxTime < 2000) // Ответ получен в последние 2 секунды
    {
      setConnectionState(CONNECTED);
    }
  }
  else if (connectionState == CONNECTED)
  {
    // 2 секунды без ответа - считаем что соединение разорвано
    if (_lastRxTime > 0 && millis() - _lastRxTime > 2000)
    {
      setConnectionState(CONNECTION_FAILED);
    }
  }
}

void WBus::process()
{
  kLineReceiver.process();

  checkConnection();
  processSerialCommands();
  processKeepAlive();

  wbusQueue.process();

  if (kLineReceiver.kLineReceivedData.isRxReceived())
  {
    if (_logging)
    {
      socketServer.sendRx(kLineReceiver.kLineReceivedData.getRxData());
      kLineReceiver.kLineReceivedData.printRx();
    }

    _lastRxTime = millis();
  }

  if (kLineReceiver.kLineReceivedData.isTxReceived())
  {
    if (_logging)
    {
      socketServer.sendTx(kLineReceiver.kLineReceivedData.getTxData());
      kLineReceiver.kLineReceivedData.printTx();
    }
  }

  socketServer.loop();
  apiServer.loop();
}