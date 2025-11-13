#include "common/queue-map/queue-map.h"

// Добавить команду в конец очереди
bool QueueMap::add(String command, std::function<void(bool, String, String)> callback, bool loop)
{
    if (_size >= QUEUE_SIZE)
    {
        Serial.println("❌ Очередь переполнена");
        return false;
    }
    _queue[_size].command = command;
    _queue[_size].callback = callback;
    _queue[_size].loop = loop;
    _size++;

    return true;
}

// Получить первую команду (без удаления)
QueueItem QueueMap::get()
{
    if (_size == 0)
    {
        QueueItem emptyItem;

        return emptyItem;
    }

    return _queue[0];
}

// Удалить первую команду со смещением остальных
QueueItem QueueMap::pop()
{
    if (_size == 0)
    {
        QueueItem emptyItem;

        return emptyItem;
    }

    // Сохраняем первую команду
    QueueItem queueItem = _queue[0];

    // Сдвигаем все элементы на одну позицию влево
    for (int i = 0; i < _size - 1; i++)
    {
        _queue[i] = _queue[i + 1];
    }

    _size--;

    return queueItem;
}

// Проверить пуста ли очередь
bool QueueMap::isEmpty()
{
    return _size == 0;
}

// Получить текущий размер очереди
int QueueMap::size()
{
    return _size;
}

// Очистить очередь
void QueueMap::clear()
{
    for (int i = 0; i < _size; i++)
    {
        _queue[i].command = String(); 
        _queue[i].callback = nullptr;
        _queue[i].loop = false;
    }
    _size = 0;
}

// Показать все элементы очереди (для отладки)
void QueueMap::print()
{
    Serial.println("📋 Содержимое очереди:");
    if (_size == 0)
    {
        Serial.println("   (пусто)");
    }
    else
    {
        for (int i = 0; i < _size; i++)
        {
            Serial.print("   ");
            Serial.print(i);
            Serial.print(": ");
            Serial.print(_queue[i].command);
            Serial.print(_queue[i].callback ? " [с колбэком]" : " [без колбэка]");
            Serial.println();
        }
    }
}