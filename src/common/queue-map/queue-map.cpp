#include "common/queue-map/queue-map.h"

// Добавить команду в конец очереди с проверкой дубликатов
bool QueueMap::add(String command, std::function<void(bool, String, String)> callback, bool loop)
{
    if (_size >= QUEUE_SIZE)
    {
        Serial.println();
        Serial.println("❌ Очередь переполнена");
        return false;
    }

    // Проверяем нет ли уже такой команды в очереди
    if (contains(command))
    {
        return false;
    }

    _queue[_size].command = command;
    _queue[_size].callback = callback;
    _queue[_size].loop = loop;
    _size++;

    return true;
}

// Проверить наличие команды в очереди
bool QueueMap::contains(String command)
{
    for (int i = 0; i < _size; i++)
    {
        if (_queue[i].command == command)
        {
            return true;
        }
    }
    return false;
}

// Найти индекс команды в очереди
int QueueMap::findCommandIndex(String command)
{
    for (int i = 0; i < _size; i++)
    {
        if (_queue[i].command == command)
        {
            return i;
        }
    }
    return -1;
}

// Удалить команду из очереди по значению
bool QueueMap::remove(String command)
{
    int index = findCommandIndex(command);
    if (index == -1)
    {
        return false;
    }
    return removeAt(index);
}

// Удалить команду из очереди по индексу
bool QueueMap::removeAt(int index)
{
    if (index < 0 || index >= _size)
    {
        Serial.println();
        Serial.println("❌ Неверный индекс для удаления: " + String(index));
        return false;
    }

    String removedCommand = _queue[index].command;

    // Сдвигаем все элементы после удаляемого на одну позицию влево
    for (int i = index; i < _size - 1; i++)
    {
        _queue[i] = _queue[i + 1];
    }

    // Очищаем последний элемент
    _queue[_size - 1].command = String();
    _queue[_size - 1].callback = nullptr;
    _queue[_size - 1].loop = false;

    _size--;

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

    // Удаляем первую команду через removeAt
    removeAt(0);

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
    Serial.println("🧹 Очередь очищена");
}

// Показать все элементы очереди (для отладки)
void QueueMap::print()
{
    Serial.println();
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
            if (_queue[i].loop)
                Serial.print(" [зациклена]");
            Serial.println();
        }
    }
}