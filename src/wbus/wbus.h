#ifndef WBUS_H
#define WBUS_H

#include <Arduino.h>
#include "common/constants.h"
#include "common/print/print.h"
#include "common/utils/utils.h"

enum WebastoState
{
    WBUS_STATE_OFF,
    WBUS_STATE_INITIALIZING,
    WBUS_STATE_READY,
    WBUS_STATE_HEATING,
    WBUS_STATE_VENTILATING,
    WBUS_STATE_ERROR
};

enum ConnectionState
{
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    CONNECTION_FAILED
};

class WBus
{
private:
    WebastoState currentState = WBUS_STATE_OFF;
    ConnectionState connectionState = DISCONNECTED;
    unsigned long lastConnectionAttempt = 0;
    bool autoReconnect = true;

public:
    void init();
    void wakeUp();
    void connect();
    void disconnect();
    void reconnect();
    
    void processQueue();
    void processReceiver();

    // Команды управления
    void startParkingHeat(int minutes = 60);
    void startVentilation(int minutes = 60);
    void startSupplementalHeat(int minutes = 60);
    void startBoostMode(int minutes = 60);
    void shutdown();
    void controlCirculationPump(bool enable);
    
    // Тестирование компонентов
    void testCombustionFan(int seconds = 10, int powerPercent = 50);
    void testFuelPump(int seconds = 5, int frequencyHz = 10);
    void testGlowPlug(int seconds = 15, int powerPercent = 75);
    void testCirculationPump(int seconds = 20, int powerPercent = 100);
    void testVehicleFan(int seconds = 8);
    void testSolenoidValve(int seconds = 12);
    void testFuelPreheating(int seconds = 25, int powerPercent = 50);

    // Статусы
    WebastoState getState() { return currentState; }
    ConnectionState getConnectionState() { return connectionState; }
    bool isConnected() { return connectionState == CONNECTED; }
    bool isHeating() { return currentState == WBUS_STATE_HEATING; }
    bool isVentilating() { return currentState == WBUS_STATE_VENTILATING; }
    
    // Управление
    void setAutoReconnect(bool enable) { autoReconnect = enable; }
    void updateConnectionState(); // для автоматического переподключения

    // Информация
    void printStatus();
};

extern WBus wBus;

#endif // WBUS_H