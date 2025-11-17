// main.cpp

#include "wbus/wbus.h"
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

const char *ap_ssid = "Webasto_WiFi";
const char *ap_password = "Epifan123";

void setup()
{
  // Инициализация пинов управления TJA1020
  wBus.init();

  Serial.begin(115200);
  Serial.println("🚗 Webasto W-Bus");
  Serial.println("=================================");
  Serial.println();

  Serial.println("📡 Запуск точки доступа...");
  Serial.println("SSID: " + String(ap_ssid));
  Serial.println("Password: " + String(ap_password));

  WiFi.mode(WIFI_AP);
  bool ap_started = WiFi.softAP(ap_ssid, ap_password);

  if (ap_started)
  {
    Serial.println("✅ Точка доступа запущена");
    Serial.println("IP адрес: " + WiFi.softAPIP().toString());
    Serial.println("MAC адрес: " + WiFi.softAPmacAddress());
  }
  else
  {
    Serial.println("❌ Ошибка запуска точки доступа");
    while (1)
    {
      delay(1000);
    } // Останавливаем выполнение
  }

  // Автоматическое пробуждение при старте
  printHelp();

  Serial.println();
  Serial.println("📱 Подключитесь с телефона к WiFi:");
  Serial.println("   Сеть: " + String(ap_ssid));
  Serial.println("   Пароль: " + String(ap_password));
  Serial.println("   Затем откройте браузер: http://" + WiFi.softAPIP().toString());
  Serial.println();
}

void loop()
{
  // Чтение и обработка пакетов W-Bus
  if (digitalRead(NSLP_PIN) == HIGH)
  {
    wBus.processQueue();
    wBus.processReceiver();
  }

  delay(1);
}