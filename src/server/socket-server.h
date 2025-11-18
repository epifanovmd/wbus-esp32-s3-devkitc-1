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

    // Буфер для хранения истории сообщений (кольцевой буфер)
    struct LogEntry
    {
        String timestamp;
        String type; // "rx", "tx", "info", "error"
        String data;
        String direction; // "in", "out"
    };
    int historyIndex = 0;
    int historyCount = 0;

    String getTimestamp();

public:
    SocketServer();

    void begin();
    void loop();

    // Основные методы отправки
    void sendRx(const String &data);
    void sendTx(const String &data);
    void sendInfo(const String &message);
    void sendError(const String &message);

    // Методы для отправки структурированных данных
    void sendSensorData(const String &sensorName, const String &value);
    void sendErrorData(const String &errorCode, const String &description);
    void sendSystemStatus(const String &status);

    // Управление
    void enable();
    void disable();
    bool isEnabled() { return enabled; }
};

extern SocketServer socketServer;

#endif //SOCKET_SERVER_H