#ifndef QUEUEMAP_H
#define QUEUEMAP_H

#include <Arduino.h>
#include <functional>

struct QueueItem {
    String command;
    std::function<void(bool success, String command, String response)> callback  = nullptr;
    bool loop = false;
};

class QueueMap {
private:
    static const int QUEUE_SIZE = 30;

    
    QueueItem _queue[QUEUE_SIZE];
    int _size = 0;

public:
    // Добавить команду в конец очереди
    bool add(String command, std::function<void(bool, String, String)> callback = nullptr, bool loop = false);
    
    // Получить первую команду (без удаления)
    QueueItem get();

    // Удалить первую команду со смещением остальных
    QueueItem pop();
    
    // Проверить пуста ли очередь
    bool isEmpty();
    
    // Получить текущий размер очереди
    int size();
    
    // Очистить очередь
    void clear();
    
    // Показать все элементы очереди (для отладки)
    void print();
};

#endif // QUEUEMAP_H