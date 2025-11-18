#ifndef WBUSQUEUE_H
#define WBUSQUEUE_H

#include <Arduino.h>
#include "common/queue-map/queue-map.h"
#include "wbus-errors-decoder.h"

enum WBusQueueState
{
    WBUS_QUEUE_IDLE_STATE,
    WBUS_QUEUE_SENDING_STATE,
    WBUS_QUEUE_WAITING_RETRY_STATE
};

class WBusQueue
{
private:
    WebastoErrorsDecoder errorsDecoder;
    QueueMap _queue;
    WBusQueueState _state = WBUS_QUEUE_IDLE_STATE;
    unsigned long _retries = 0;
    unsigned long _maxRetries = 10;

    void _sendCurrentCommand();
    void _completeCurrentCommand(String response);
    void _handleRepeat();

public:
    bool add(String command, std::function<void(String, String)> callback = nullptr, bool loop = false);
    bool addPriority(String command, std::function<void(String, String)> callback = nullptr, bool loop = false);
    void setInterval(unsigned long interval);
    void setMaxRetries(unsigned long retries);
    void setTimeout(unsigned long timeout);

    bool removeCommand(String command);
    bool containsCommand(String command);
    void clear();
    void process();
    void processNakResponse(const String response);

    bool isEmpty() { return _queue.isEmpty(); }
    void printQueue();
};

extern WBusQueue wbusQueue;

#endif // WBUSQUEUE_H