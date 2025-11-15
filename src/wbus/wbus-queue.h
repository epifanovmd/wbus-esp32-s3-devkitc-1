#ifndef WBUSQUEUE_H
#define WBUSQUEUE_H

#include <Arduino.h>
#include "common/queue-map/queue-map.h"
#include "wbus/wbus-error-codes.h"

enum WBusQueueState
{
    WBUS_QUEUE_IDLE_STATE,
    WBUS_QUEUE_SENDING_STATE,
    WBUS_QUEUE_WAITING_RETRY_STATE
};

class WBusQueue
{
private:
    WebastoErrorCodes errorCodes;
    QueueMap _queue;
    WBusQueueState _state = WBUS_QUEUE_IDLE_STATE;
    unsigned long _processDelay = 0;
    unsigned long _lastProcessTime = 0;
    unsigned long _lastSendTime = 0;
    unsigned long _timeout = 1000;
    unsigned long _retryDelay = 300;
    unsigned long _retries = 0;
    unsigned long _maxRetries = 10;

    void _sendCurrentCommand();
    void _completeCurrentCommand(String response, bool success = true);
    void _handleRepeat();

public:
    bool add(String command, std::function<void(bool, String, String)> callback = nullptr, bool loop = false);
    void setProcessDelay(unsigned long processDelay);
    void setRepeatDelay(unsigned long retryDelay);
    void setMaxRetries(unsigned long retries);

    void clear();
    void process();
    void processNakResponse(const String response);
};

extern WBusQueue wbusQueue;

#endif // WBUSQUEUE_H