#pragma once
#include "../domain/Entities.h"

class IHeaterController {
public:
    virtual ~IHeaterController() = default;
    
    virtual void initialize() = 0;
    virtual HeaterStatus getStatus() const = 0;
    
    // Управление подключением
    virtual void connect() = 0;
    virtual void disconnect() = 0;
    
    // Основные команды управления
    virtual void startParkingHeat(int minutes = 60) = 0;
    virtual void startVentilation(int minutes = 60) = 0;
    virtual void startSupplementalHeat(int minutes = 60) = 0;
    virtual void startBoostMode(int minutes = 60) = 0;
    virtual void controlCirculationPump(bool enable) = 0;
    virtual void shutdown() = 0;
    
    // Тестирование компонентов
    virtual void testCombustionFan(int seconds, int powerPercent) = 0;
    virtual void testFuelPump(int seconds, int frequencyHz) = 0;
    virtual void testGlowPlug(int seconds, int powerPercent) = 0;
    virtual void testCirculationPump(int seconds, int powerPercent) = 0;
    virtual void testVehicleFan(int seconds) = 0;
    virtual void testSolenoidValve(int seconds) = 0;
    virtual void testFuelPreheating(int seconds, int powerPercent) = 0;
};