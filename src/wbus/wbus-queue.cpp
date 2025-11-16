#include "wbus-queue.h"
#include "wbus-sender.h"
#include "wbus-error-codes.h"
#include "receiver/wbus-receiver.h"

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
        if (wBusReceiver.wBusReceivedData.isRxReceived())
        {
            // ✅ Ответ получен
            _completeCurrentCommand(wBusReceiver.wBusReceivedData.rxString);
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

void WBusQueue::processNakResponse(const String response) {
    if (!errorCodes.isNakResponse(response)) return;
    
    // Парсим NAK пакет
    String cleanResponse = response;
    cleanResponse.replace(" ", "");
    
    if (cleanResponse.length() >= 10) {
        byte failedCommand = strtoul(cleanResponse.substring(8, 10).c_str(), NULL, 16);
        byte errorCode = strtoul(cleanResponse.substring(10, 12).c_str(), NULL, 16);
        
        Serial.println();
        Serial.println("❌ NAK получен:");
        Serial.println("   Невыполненная команда: 0x" + String(failedCommand, HEX));
        Serial.println("   Код ошибки: 0x" + String(errorCode, HEX));
        
        // Декодируем причину ошибки
        errorCodes.decodeNakError(failedCommand, errorCode);
    }
}

void WBusQueue::_completeCurrentCommand(String response, bool success)
{
    if (errorCodes.isNakResponse(response))
    {
        success = false;
        processNakResponse(response);
    }

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