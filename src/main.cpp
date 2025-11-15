// main.cpp

#include "common/tja1020/tja1020.h"
#include "wbus/wbus.h"

void setup()
{
  // Инициализация пинов управления TJA1020
  initTJA1020();

  Serial.begin(115200);
  Serial.println("🚗 Webasto W-Bus");
  Serial.println("=================================");
  Serial.println();

  // Автоматическое пробуждение при старте
  wakeUpTJA1020();
  printHelp();
}

void loop()
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
      connectToWebasto();
    }
    else if (command == "disconnect")
    {
      wbusQueue.clear();
    }
    else if (command == "errors" || command == "err")
    {
      webastoError.check();
    }
    else if (command == "clear" || command == "clr")
    {
      webastoError.clear();
    }

    else if (command == "i")
    {
      webastoInfo.getWBusVersion();
      webastoInfo.getDeviceName();
      webastoInfo.getWBusCode();
      webastoInfo.getDeviceID();
      webastoInfo.getHeaterManufactureDate();
      webastoInfo.getControllerManufactureDate();
      webastoInfo.getCustomerID();
      webastoInfo.getSerialNumber();
    }
    else if (command == "help" || command == "h")
    {
      printHelp();
    }
    else if (command == "break")
    {
      wakeUpWebasto();
    }
    else
    {
      sendWbusCommand(command);
    }
  }

  wbusQueue.process();

  // Чтение и обработка пакетов W-Bus
  if (digitalRead(NSLP_PIN) == HIGH)
  {
    readWBusData();
  }

  delay(1);
}