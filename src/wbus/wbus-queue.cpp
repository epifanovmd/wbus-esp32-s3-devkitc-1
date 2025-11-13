#include "wbus/wbus.h"
#include "wbus/wbus-queue.h"
#include "wbus/wbus-sender.h"

WBusQueue wbusQueue;

bool WBusQueue::add(String command, std::function<void(bool, String, String)> callback, bool loop)
{
    WBusPacket packet = parseHexStringToPacket(command);

    if (!validateWbusPacket(packet))
    {
        return false;
    }

    return _queue.add(command, callback, loop);
}

void WBusQueue::setProcessDelay(unsigned long processDelay)
{
    _processDelay = processDelay;
}

void WBusQueue::setRepeatDelay(unsigned long retryDelay)
{
    _retryDelay = retryDelay;
}

void WBusQueue::setMaxRetries(unsigned long retries)
{
    _maxRetries = retries;
}

void WBusQueue::clear()
{
    _queue.clear();
    _state = WBUS_QUEUE_IDLE_STATE;
    _retries = 0;
}

void WBusQueue::process()
{
    switch (_state)
    {
    case WBUS_QUEUE_IDLE_STATE:
        if (!_queue.isEmpty())
        {
            if (_processDelay > 0 && millis() - _lastProcessTime < _processDelay)
            {
                break;
            }
            _lastProcessTime = millis();

            _sendCurrentCommand();
        }
        break;

    case WBUS_QUEUE_SENDING_STATE:
        if (wBusReceivedData.isRxReceived())
        {
            // ✅ Ответ получен
            _completeCurrentCommand(wBusReceivedData.rxString);
        }
        else if (millis() - _lastSendTime > _timeout)
        {
            // ⏰ Таймаут
            _handleRepeat();
        }
        break;

    case WBUS_QUEUE_WAITING_RETRY_STATE:
        if (millis() - _lastSendTime > _retryDelay)
        {
            _sendCurrentCommand();
        }
        break;
    }
}

void WBusQueue::_sendCurrentCommand()
{
    String command = _queue.get().command;

    _lastSendTime = millis();
    _state = WBUS_QUEUE_SENDING_STATE;

    sendWbusCommand(command);
}

void WBusQueue::_completeCurrentCommand(String response, bool success)
{
    _state = WBUS_QUEUE_IDLE_STATE;
    _retries = 0;
    QueueItem queueItem = _queue.pop();

    if (queueItem.loop && success)
    {
        _queue.add(queueItem.command, queueItem.callback, queueItem.loop);
    }

    if (queueItem.callback != nullptr)
    {
        queueItem.callback(success, queueItem.command, response);
    }

    if (_queue.isEmpty())
    {
        Serial.println();
        Serial.print("❌ Очередь команд пуста");
    }
}

void WBusQueue::_handleRepeat()
{
    _retries++;
    Serial.println();
    Serial.print("❌ Попытка " + String(_retries));
    if (_retries >= _maxRetries)
    {
        Serial.println();
        Serial.print("❌ Все попытки исчерпаны");
        _completeCurrentCommand("", false);
    }
    else
    {
        _state = WBUS_QUEUE_WAITING_RETRY_STATE;
        _lastSendTime = millis();
    }
}