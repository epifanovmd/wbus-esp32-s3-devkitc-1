// wbus.cpp

#include "wbus/wbus.h"
#include "wbus/receiver/wbus-receiver.h"
#include "common/tja1020/tja1020.h"
#include "wbus/wbus-sensors.h"
#include "wbus/wbus-info.h"
#include "wbus/wbus-error.h"
#include "wbus/wbus-sender.h"

WBus wBus;

void WBus::init()
{
  initTJA1020();
  wakeUpTJA1020();
}

void WBus::wakeUp()
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

void WBus::connect()
{
  wakeUp();

  delay(100);
  Serial.println("🔌 Подключение к Webasto...");

  webastoInfo.getMainInfo();

  wbusQueue.add(
      CMD_DIAGNOSTIC,
      [this](bool success, String cmd, String response)
      {
        Serial.println();

        if (success)
        {
          Serial.print("✅ Подключение прошло успешно");
          wbusQueue.setProcessDelay(150);

          webastoInfo.getAllInfo();
          webastoSensors.getOperationalInfo();
          webastoSensors.getFuelSettings();
          webastoSensors.getOnOffFlags();
          webastoSensors.getStatusFlags();
          webastoSensors.getOperatingState();
        }
        else
        {
          Serial.print("❌ Не удалось подключиться!");
        }
        Serial.println();
      });
}

void WBus::processQueue()
{
  // Проверяем команды от пользователя
  if (Serial.available())
  {
    String command = Serial.readString();
    command.trim();

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
      wBus.connect();
    }
    else if (command == "disconnect")
    {
      wbusQueue.clear();
    }
    else if (command == "info" || command == "i")
    {
      webastoInfo.printInfo();
    }
    else if (command == "errors" || command == "err")
    {
      webastoError.check();
    }
    else if (command == "clear" || command == "clr")
    {
      webastoError.clear();
    }
    else if (command == "help" || command == "h")
    {
      printHelp();
    }
    else if (command == "break")
    {
      wBus.wakeUp();
    }
    else
    {
      sendWbusCommand(command);
    }
  }

  wbusQueue.process();
}

void WBus::processReceiver()
{
  wBusReceiver.process();
}