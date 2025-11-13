// utils.h

#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>
#include "common/constants.h"

byte calculateChecksum(byte* data, int length);
bool isHexString(String str);

// Класс для неблокирующей блокировки
class NonBlockingLocker {
private:
    unsigned long _lockTime;
    unsigned long _timeout;
    bool _isLocked;
    
public:
    NonBlockingLocker();
    void lock(unsigned long timeout = 0);
    void free();
    bool isActive();
    bool isTimeout();
    unsigned long getRemainingTime();
};

class PacketQueue {
private:
    String _queue[50];
    int _queueSize;
    int _currentIndex;
    
public:
    PacketQueue();
    bool add(String command);
    String pop();
    bool isEmpty();
    int size();
    void clear();
};


// Класс для управления таймаутами и попытками
class TimeoutRetries {
private:
    unsigned long _startTime;
    unsigned long _timeout;
    byte _maxRetries;
    byte _currentAttempt;
    bool _active;
    
public:
    TimeoutRetries();
    void start(unsigned long timeout, byte retries = 1);
    void stop();
    bool allow();
    byte getCurrentAttempt();
    byte getRemainingAttempts();
    unsigned long getRemainingTime();
    bool isActive();
};

#endif // UTILS_H