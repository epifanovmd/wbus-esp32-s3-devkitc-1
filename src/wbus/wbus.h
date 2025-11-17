#ifndef WBUS_H
#define WBUS_H

#include <Arduino.h>
#include "common/constants.h"
#include "common/print/print.h"
#include "common/utils/utils.h"
#include "wbus-sensors.h"

enum WebastoState
{
    WBUS_STATE_OFF,          // Выключен
    WBUS_STATE_READY,        // Готов к работе
    WBUS_STATE_PARKING_HEAT, // Паркинг-нагрев
    WBUS_STATE_VENTILATION,  // Вентиляция
    WBUS_STATE_SUPP_HEAT,    // Дополнительный нагрев
    WBUS_STATE_BOOST,        // Boost режим
    WBUS_STATE_CIRC_PUMP,    // Только циркуляционный насос
    WBUS_STATE_STARTUP,      // Запуск
    WBUS_STATE_SHUTDOWN,     // Выключение
    WBUS_STATE_ERROR         // Ошибка
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
    unsigned long lastStateUpdate = 0;
    unsigned long _lastKeepAliveTime = 0;
    unsigned long _lastRxTime = 0;

    void checkConnectionTimeout();
    void updateStateFromSensors();
    WebastoState determineStateFromFlags(const StatusFlags &flags, OnOffFlags &onOff);
    String getKeepAliveCommandForCurrentState();

public:
    void init();
    void wakeUp();
    void connect();
    void disconnect();

    void processQueue();
    void processKeepAlive();
    void processReceiver();
    void updateState();

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
        String getStateName();
    WebastoState getState() { return currentState; }
    ConnectionState getConnectionState() { return connectionState; }
    bool isConnected() { return connectionState == CONNECTED; }

    // Управление
    void setAutoReconnect(bool enable) { autoReconnect = enable; }

    // Информация
    void printStatus();
};

extern WBus wBus;

#endif // WBUS_H