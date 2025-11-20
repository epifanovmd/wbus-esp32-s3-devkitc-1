#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#include <Arduino.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

class SocketServer
{
private:
    WebSocketsServer webSocket;
    bool enabled = false;

    int historyIndex = 0;
    int historyCount = 0;

public:
    SocketServer();

    void begin();
    void loop();

    // Основные методы отправки
    void sendRx(const String &data);
    void sendTx(const String &data);
    void sendInfo(const String &message);
    void send(const String &type, const String &json);
    void sendSystemStatus(const String &type, const String &currentState, const String &prevState);

    // Управление
    void enable();
    void disable();
    bool isEnabled() { return enabled; }
};

extern SocketServer socketServer;

#endif //SOCKET_SERVER_H