#include "./Timer.h"

Timer::Timer(unsigned long intervalMs, bool autoReset)
{
    _interval = intervalMs;
    _autoReset = autoReset;
    _lastExecution = millis();
}

void Timer::setInterval(unsigned long intervalMs)
{
    _interval = intervalMs;
    reset();
}

void Timer::reset()
{
    _lastExecution = millis();
}

bool Timer::isReady()
{
    unsigned long currentTime = millis();
    unsigned long elapsed = currentTime - _lastExecution;

    if (elapsed >= _interval)
    {
        if (_autoReset)
        {
            _lastExecution = currentTime;
        }
        return true;
    }
    return false;
}

void Timer::forceReady()
{
    _lastExecution = millis() - _interval;
}

unsigned long Timer::getRemainingTime()
{
    unsigned long currentTime = millis();
    unsigned long elapsed = currentTime - _lastExecution;

    if (elapsed >= _interval)
    {
        return 0;
    }
    return _interval - elapsed;
}

unsigned long Timer::getElapsedTime()
{
    return millis() - _lastExecution;
}

unsigned long Timer::getCurrentInterval()
{
    return _interval;
}