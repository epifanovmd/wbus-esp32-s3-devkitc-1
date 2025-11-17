#include "common/timeout/timeout.h"

Timeout::Timeout(unsigned long intervalMs, bool autoReset)
{
    _interval = intervalMs;
    _autoReset = autoReset;
    _lastExecution = millis();
}

void Timeout::setInterval(unsigned long intervalMs)
{
    _interval = intervalMs;
    reset();
}

void Timeout::reset()
{
    _lastExecution = millis();
}

bool Timeout::isReady()
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

void Timeout::forceReady()
{
    _lastExecution = millis() - _interval;
}

unsigned long Timeout::getRemainingTime()
{
    unsigned long currentTime = millis();
    unsigned long elapsed = currentTime - _lastExecution;

    if (elapsed >= _interval)
    {
        return 0;
    }
    return _interval - elapsed;
}

unsigned long Timeout::getElapsedTime()
{
    return millis() - _lastExecution;
}