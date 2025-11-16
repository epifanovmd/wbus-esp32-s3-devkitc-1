#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include "wbus/wbus-sensors.h"
#include "wbus/wbus-info.h"

class WebastoWebServer {
private:
    WebServer server;
    WebSocketsServer webSocket;
    
    // Данные для отправки
    String currentData;
    unsigned long lastBroadcast;
    unsigned long broadcastInterval;
    
    // HTML страница
    String webPage;
    String escapeJSON(String input);

public:
    WebastoWebServer();
    void begin();
    void handleClient();
    void broadcastData(String data);
    void sendSensorData();
    void sendDeviceInfo();
    
    // Обработчики
    void handleRoot();
    void handleData();
    void handleDeviceInfo();
    void handleWebSocket(uint8_t num, WStype_t type, uint8_t * payload, size_t length);
    
    // Генерация HTML
    String generateHTML();
};

extern WebastoWebServer webServer;

#endif // WEB_SERVER_H