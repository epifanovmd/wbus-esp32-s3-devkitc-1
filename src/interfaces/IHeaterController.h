#pragma once
#include "../domain/Entities.h"

class IHeaterController {
public:
    virtual ~IHeaterController() = default;
    
    virtual bool initialize() = 0;
    virtual HeaterStatus getStatus() const = 0;
    
    // Управление подключением
    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    
    // Основные команды управления
    virtual bool startParkingHeat(int minutes = 60) = 0;
    virtual bool startVentilation(int minutes = 60) = 0;
    virtual bool startSupplementalHeat(int minutes = 60) = 0;
    virtual bool startBoostMode(int minutes = 60) = 0;
    virtual bool controlCirculationPump(bool enable) = 0;
    virtual bool shutdown() = 0;
    
    // Тестирование компонентов
    virtual bool testCombustionFan(int seconds, int powerPercent) = 0;
    virtual bool testFuelPump(int seconds, int frequencyHz) = 0;
    virtual bool testGlowPlug(int seconds, int powerPercent) = 0;
    virtual bool testCirculationPump(int seconds, int powerPercent) = 0;
    virtual bool testVehicleFan(int seconds) = 0;
    virtual bool testSolenoidValve(int seconds) = 0;
    virtual bool testFuelPreheating(int seconds, int powerPercent) = 0;
};