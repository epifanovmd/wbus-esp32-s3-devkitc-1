#include "wbus-queue.h"
#include "wbus-sender.h"
#include "common/timeout/timeout.h"
#include "kline-receiver/kline-receiver.h"

Timeout queueTimer(150);
Timeout timeoutTimer(2000, false);

WBusQueue wbusQueue;

bool WBusQueue::add(String command, std::function<void(String, String)> callback, bool loop)
{
    WBusPacket packet = parseHexStringToPacket(command);

    if (!validateWbusPacket(packet))
    {
        return false;
    }

    return _queue.add(command, callback, loop);
}

bool WBusQueue::addPriority(String command, std::function<void(String, String)> callback, bool loop)
{
    WBusPacket packet = parseHexStringToPacket(command);

    if (!validateWbusPacket(packet))
    {
        return false;
    }

    return _queue.addPriority(command, callback, loop);
}

void WBusQueue::setInterval(unsigned long interval)
{
    queueTimer.setInterval(interval);
}

void WBusQueue::setMaxRetries(unsigned long retries)
{
    _maxRetries = retries;
}

void WBusQueue::setTimeout(unsigned long timeout)
{
    timeoutTimer.setInterval(timeout);
    Serial.println();
    Serial.println("⏰ Таймаут установлен: " + String(timeout) + "мс");
}

bool WBusQueue::removeCommand(String command)
{
    return _queue.remove(command);
}

bool WBusQueue::containsCommand(String command)
{
    return _queue.contains(command);
}

void WBusQueue::printQueue()
{
    _queue.print();
}

void WBusQueue::clear()
{
    _queue.clear();
    _state = WBUS_QUEUE_IDLE_STATE;
    _retries = 0;
}

void WBusQueue::process()
{
    // Обработка обычной очереди
    switch (_state)
    {
    case WBUS_QUEUE_IDLE_STATE:
        if (!_queue.isEmpty())
        {
            if (queueTimer.isReady())
            {
                _sendCurrentCommand();
            }
        }
        break;

    case WBUS_QUEUE_SENDING_STATE:
        if (kLineReceiver.kLineReceivedData.isRxReceived())
        {
            // ✅ Ответ получен
            _completeCurrentCommand(kLineReceiver.kLineReceivedData.rxString);
        }
        else if (timeoutTimer.isReady())
        {
            // ⏰ Таймаут
            _handleRepeat();
        }
        break;

    case WBUS_QUEUE_WAITING_RETRY_STATE:

        // BREAK set - удерживаем линию в LOW 50ms
        KLineSerial.write(0x00);
        delay(50);

        // BREAK reset - отпускаем линию и ждем 50ms
        KLineSerial.flush();
        delay(50);
        _sendCurrentCommand();
        break;
    }
}

void WBusQueue::_sendCurrentCommand()
{
    String command = _queue.get().command;

    timeoutTimer.reset();
    _state = WBUS_QUEUE_SENDING_STATE;

    sendWbusCommand(command);
}

void WBusQueue::processNakResponse(const String response)
{
    if (!errorsDecoder.isNakResponse(response))
        return;

    // Парсим NAK пакет
    String cleanResponse = response;
    cleanResponse.replace(" ", "");

    if (cleanResponse.length() >= 10)
    {
        byte failedCommand = strtoul(cleanResponse.substring(8, 10).c_str(), NULL, 16);
        byte errorCode = strtoul(cleanResponse.substring(10, 12).c_str(), NULL, 16);

        Serial.println();
        Serial.println("❌ NAK получен:");
        Serial.println("   Невыполненная команда: 0x" + String(failedCommand, HEX));
        Serial.println("   Код ошибки: 0x" + String(errorCode, HEX));

        // Декодируем причину ошибки
        errorsDecoder.decodeNakError(failedCommand, errorCode);
    }
}

void WBusQueue::_completeCurrentCommand(String response)
{
    if (errorsDecoder.isNakResponse(response))
    {
        processNakResponse(response);
    }

    _state = WBUS_QUEUE_IDLE_STATE;
    _retries = 0;
    QueueItem queueItem = _queue.pop();

    if (queueItem.loop && !response.isEmpty())
    {
        _queue.add(queueItem.command, queueItem.callback, queueItem.loop);
    }

    if (queueItem.callback != nullptr)
    {
        queueItem.callback(queueItem.command, response);
    }

    if (_queue.isEmpty())
    {
        // Serial.println();
        // Serial.print("❌ Очередь команд пуста");
    }
}

void WBusQueue::_handleRepeat()
{
    _retries++;
    Serial.println();
    Serial.print("❌ Попытка " + String(_retries));
    if (_retries >= _maxRetries)
    {
        _completeCurrentCommand("");
        clear();
    }
    else
    {
        _state = WBUS_QUEUE_WAITING_RETRY_STATE;
    }
}