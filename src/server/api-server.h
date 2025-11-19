// api-server.h
#ifndef API_SERVER_H
#define API_SERVER_H

#include <Arduino.h>
#include <WebServer.h>
#include <LittleFS.h>

class ApiServer
{
private:
    WebServer server;
    
    // Методы для обслуживания статических файлов
    void serveHTML();
    void serveHTMLFromCode();
    void serveStaticFile(String path, String contentType);

public:
    ApiServer();
    void begin();
    void loop();

private:
    void handleGetSystemState();
    void handleGetDeviceInfo();
    void handleGetSensorsData();
    void handleGetErrors();
    void handleGetAllData();
    
    // Endpoint-ы управления
    void handleConnect();
    void handleDisconnect();
    void handleStartParkingHeat();
    void handleStopHeater();
    void handleToggleLogging();

    void handleNotFound();
};

extern ApiServer apiServer;

#endif // API_SERVER_H