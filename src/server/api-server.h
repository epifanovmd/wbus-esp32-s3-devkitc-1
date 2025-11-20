#ifndef API_SERVER_H
#define API_SERVER_H

#include <Arduino.h>
#include <WebServer.h>
#include <LittleFS.h>

class ApiServer
{
private:
    WebServer server;

    // Методы инициализации
    void initializeFileSystem();
    void listFilesystemContents();
    void setupEndpoints();
    void setupTestEndpoints();
    void printAvailableEndpoints();

    // Методы обслуживания файлов
    void serveFallbackHTML();

    // Handlers для тестирования компонентов
    void handleTestCombustionFan();
    void handleTestFuelPump();
    void handleTestGlowPlug();
    void handleTestCirculationPump();
    void handleTestVehicleFan();
    void handleTestSolenoidValve();
    void handleTestFuelPreheating();

public:
    ApiServer();
    void begin();
    void loop();

    // Основные методы
    void serveHTML();
    void serveStaticFile(String path, String contentType);

    // Handlers для управления режимами
    void handleConnect();
    void handleDisconnect();
    void handleStartParkingHeat();
    void handleStartVentilation();
    void handleStartSupplementalHeat();
    void handleStartBoostMode();
    void handleControlCirculationPump();
    void handleStopHeater();
    void handleToggleLogging();

    // Handlers для данных
    void handleGetSystemState();
    void handleGetDeviceInfo();
    void handleGetSensorsData();
    void handleGetErrors();
    void handleGetAllData();
    void handleNotFound();
};

extern ApiServer apiServer;

#endif // API_SERVER_H