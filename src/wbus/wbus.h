#ifndef WBUS_H
#define WBUS_H

#include <Arduino.h>
#include "common/constants.h"
#include "common/print/print.h"
#include "common/utils/utils.h"
#include "wbus-sensors.h"
#include "wbus-info.h"
#include "wbus-errors.h"

#define RGB_PIN RGB_BUILTIN

extern String WebastoStateNames[];
extern String ConnectionStateNames[];

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
    unsigned long _lastRxTime = 0;
    bool _logging = false;

    WebastoState determineStateFromFlags(const StatusFlags *flags, OnOffFlags *onOff);
    String getKeepAliveCommandForCurrentState();
    void processSerialCommands();
    void processKeepAlive();
    void updateStateFromSensors(std::function<void()> callback = nullptr);
    void checkConnection();
    void setConnectionState(ConnectionState newState);
    void setState(WebastoState newState);

public:
    void init();
    void wakeUp();
    void connect(std::function<void(String tx, String rx)> callback = nullptr);
    void disconnect();

    void process();

    // Команды управления с колбэками
    void shutdown(std::function<void(String tx, String rx)> callback = nullptr);
    void startParkingHeat(int minutes = 60, std::function<void(String tx, String rx)> callback = nullptr);
    void startVentilation(int minutes = 60, std::function<void(String tx, String rx)> callback = nullptr);
    void startSupplementalHeat(int minutes = 60, std::function<void(String tx, String rx)> callback = nullptr);
    void startBoostMode(int minutes = 60, std::function<void(String tx, String rx)> callback = nullptr);
    void controlCirculationPump(bool enable, std::function<void(String tx, String rx)> callback = nullptr);

    // Тестирование компонентов с колбэками
    void testCombustionFan(int seconds = 10, int powerPercent = 50, std::function<void(String tx, String rx)> callback = nullptr);
    void testFuelPump(int seconds = 5, int frequencyHz = 10, std::function<void(String tx, String rx)> callback = nullptr);
    void testGlowPlug(int seconds = 15, int powerPercent = 75, std::function<void(String tx, String rx)> callback = nullptr);
    void testCirculationPump(int seconds = 20, int powerPercent = 100, std::function<void(String tx, String rx)> callback = nullptr);
    void testVehicleFan(int seconds = 8, std::function<void(String tx, String rx)> callback = nullptr);
    void testSolenoidValve(int seconds = 12, std::function<void(String tx, String rx)> callback = nullptr);
    void testFuelPreheating(int seconds = 25, int powerPercent = 50, std::function<void(String tx, String rx)> callback = nullptr);

    // Статусы
    String getStateName(WebastoState state);
    String getCurrentStateName();
    WebastoState getState() { return currentState; }
    ConnectionState getConnectionState() { return connectionState; }
    bool isConnected() { return connectionState == CONNECTED; }
    bool isLogging() { return _logging; }

    // Логирование
    void startLogging() { _logging = true; }
    void stopLogging() { _logging = false; }
};

extern WBus wBus;

#endif // WBUS_H