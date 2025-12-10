#ifndef TIMER_H
#define TIMER_H

#include <Arduino.h>

class Timer
{
private:
    unsigned long _interval;      // Интервал в миллисекундах
    unsigned long _lastExecution; // Время последнего выполнения
    bool _autoReset;              // Автоматически сбрасывать таймер

public:
    // Конструктор с интервалом и автосбросом
    Timer(unsigned long intervalMs, bool autoReset = true);

    void setInterval(unsigned long intervalMs);
    void reset();
    bool isReady();
    void forceReady();
    unsigned long getRemainingTime();
    unsigned long getElapsedTime();
    unsigned long getCurrentInterval();
};

#endif // TIMER_H