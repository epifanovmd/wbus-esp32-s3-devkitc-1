#ifndef TIMEOUT_H
#define TIMEOUT_H

#include <Arduino.h>

class Timeout
{
private:
    unsigned long _interval;      // Интервал в миллисекундах
    unsigned long _lastExecution; // Время последнего выполнения
    bool _autoReset;              // Автоматически сбрасывать таймер

public:
    // Конструктор с интервалом и автосбросом
    Timeout(unsigned long intervalMs, bool autoReset = true);

    // Установить новый интервал
    void setInterval(unsigned long intervalMs);

    // Перезапустить таймер (сбросить время последнего выполнения)
    void reset();

    // Проверить, готов ли таймер к выполнению
    bool isReady();

    // Принудительно установить состояние "готов"
    void forceReady();

    // Получить оставшееся время до готовности
    unsigned long getRemainingTime();

    // Получить прошедшее время с последнего выполнения
    unsigned long getElapsedTime();
};

#endif // TIMEOUT_H