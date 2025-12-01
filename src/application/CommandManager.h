#pragma once
#include <Arduino.h>
#include <functional>
#include <queue>
#include <vector>
#include <deque>
#include "./CommandReceiver.h"
#include "../common/Timer.h"
#include "../core/EventBus.h"
#include "../core/ConfigManager.h"
#include "../infrastructure/protocol/WBusCommand.h"
#include "../infrastructure/protocol/WBusErrorsDecoder.h"
#include "../interfaces/IBusManager.h"
#include "../domain/Events.h"

// Состояния обработки (аналог WBusQueueState из оригинала)
enum class ProcessingState
{
    IDLE,       // Ожидание команды
    SENDING,    // Команда отправлена, ждем ответ
    RETRY,      // Повторная отправка
    BREAK_SET,  // BREAK сигнал установлен
    BREAK_RESET // BREAK сигнал сброшен
};

Timer queueTimer(100);
Timer timeoutTimer(2000, false);
Timer breakTimer(50, false);

struct Command
{
    String data;
    std::function<void(String tx, String rx)> callback;
    bool loop = false;

    Command() : data(""), callback(nullptr) {}
    Command(const String &cmd, std::function<void(String, String)> cb = nullptr, bool lp = false)
        : data(cmd), callback(cb), loop(lp) {}
};

class CommandManager
{
private:
    std::queue<Command> priorityQueue; // Приоритетная очередь
    std::queue<Command> normalQueue;   // Обычная очередь

    EventBus &eventBus;
    const BusConfig &config;
    IBusManager &busManager;
    CommanReceiver &commanReceiver;
    WBusErrorsDecoder errorsDecoder;

    Command processingCommand;
    ProcessingState state = ProcessingState::IDLE;
    uint8_t currentRetries = 0;
    const uint8_t MAX_RETRIES = 5;

public:
    CommandManager(EventBus &bus, IBusManager &busMngr, CommanReceiver &receiver)
        : eventBus(bus), config(ConfigManager::getInstance().getConfig().bus), commanReceiver(receiver), busManager(busMngr)
    {
    }

    bool addCommand(const String &command, std::function<void(String, String)> callback = nullptr,
                    bool loop = false)
    {
        if (getTotalQueueSize() >= 30 || containsCommand(command))
            return false;

        normalQueue.push(Command(command, callback, loop));
        return true;
    }

    bool addPriorityCommand(const String &command, std::function<void(String, String)> callback = nullptr, bool loop = false)
    {
        if (getTotalQueueSize() >= 30 || containsPriorityCommand(command))
            return false;

        priorityQueue.push(Command(command, callback, loop));
        return true;
    }

    void process()
    {
        switch (state)
        {
        case ProcessingState::IDLE:
            if (!isQueueEmpty() && queueTimer.isReady())
            {
                processingCommand = getNextCommand();
                sendCurrentCommand();
            }
            break;

        case ProcessingState::SENDING:
            if (commanReceiver.isRxReceived())
            {
                // ✅ Ответ получен
                complete(commanReceiver.getRxData());
            }
            else if (timeoutTimer.isReady())
            {
                // ⏰ Таймаут
                handleTimeout();
            }
            break;

        case ProcessingState::RETRY:
            if (breakTimer.isReady())
            {
                sendCurrentCommand();
            }
            break;

        case ProcessingState::BREAK_SET:
            // BREAK set - удерживаем линию в LOW 50ms
            busManager.sendBreakSignal(true);
            breakTimer.reset();
            state = ProcessingState::BREAK_RESET;
            break;

        case ProcessingState::BREAK_RESET:
            if (breakTimer.isReady())
            {
                // BREAK reset - отпускаем линию и ждем 50ms
                busManager.sendBreakSignal(false);
                breakTimer.reset();
                state = ProcessingState::RETRY;
            }
            break;
        }
    }

    bool containsCommand(const String &command)
    {
        std::queue<Command> tempQueue = normalQueue;
        while (!tempQueue.empty())
        {
            if (tempQueue.front().data == command)
                return true;
            tempQueue.pop();
        }
        return false;
    }

    bool containsPriorityCommand(const String &command)
    {
        std::queue<Command> tempQueue = priorityQueue;
        while (!tempQueue.empty())
        {
            if (tempQueue.front().data == command)
                return true;
            tempQueue.pop();
        }
        return false;
    }

    void clear()
    {
        while (!normalQueue.empty())
        {
            normalQueue.pop();
        }
        while (!priorityQueue.empty())
        {
            priorityQueue.pop();
        }

        state = ProcessingState::IDLE;
        currentRetries = 0;

        Serial.println();
        Serial.println("🧹 Очередь очищена");
    }

    void setInterval(unsigned long interval)
    {
        queueTimer.setInterval(interval);
    }

    void setTimeout(unsigned long timeout)
    {
        timeoutTimer.setInterval(timeout);
    }

    bool isEmpty() const
    {
        return isQueueEmpty() && state == ProcessingState::IDLE;
    }

    size_t getTotalQueueSize() const
    {
        return priorityQueue.size() + normalQueue.size();
    }

private:
    bool isQueueEmpty() const
    {
        return priorityQueue.empty() && normalQueue.empty();
    }

    Command getNextCommand()
    {
        if (!priorityQueue.empty())
        {
            Command cmd = priorityQueue.front();
            priorityQueue.pop();
            return cmd;
        }
        else if (!normalQueue.empty())
        {
            Command cmd = normalQueue.front();
            normalQueue.pop();
            return cmd;
        }

        return Command();
    }

    void sendCurrentCommand()
    {
        WBusCommand wBusCommand(processingCommand.data);

        if (!wBusCommand.isValid())
            return;

        if (busManager.sendCommand(wBusCommand.data, wBusCommand.byteCount))
        {
            state = ProcessingState::SENDING;
            timeoutTimer.reset();
            eventBus.publish(EventType::COMMAND_SENT, processingCommand.data);
        }
        else
        {
            complete();
        }
    }

    void complete(const String &response = "")
    {
        if (processingCommand.callback)
        {
            processingCommand.callback(processingCommand.data, response);
        }

        if (!response.isEmpty())
        {
            if (processingCommand.loop)
            {
                normalQueue.push(Command(processingCommand.data, processingCommand.callback, processingCommand.loop));
            }
            eventBus.publish<CommandReceivedEvent>(EventType::COMMAND_RECEIVED, {processingCommand.data, response});
        }
        else
        {
            eventBus.publish(EventType::COMMAND_SENT_ERRROR, processingCommand.data);
        }

        state = ProcessingState::IDLE;
        currentRetries = 0;
        processingCommand = Command();
    }

    void handleTimeout()
    {
        currentRetries++;

        if (currentRetries > MAX_RETRIES)
        {
            complete();
            // Не очищаем все очереди, только сбрасываем состояние
            state = ProcessingState::IDLE;
            currentRetries = 0;
        }
        else
        {
            eventBus.publish<ConnectionTimeoutEvent>(EventType::COMMAND_SENT_TIMEOUT, {currentRetries, processingCommand.data});
            Serial.println();
            Serial.println("🔄 Повторная отправка " + String(currentRetries) + "/" + String(MAX_RETRIES) + ": " + processingCommand.data);

            state = ProcessingState::BREAK_SET;
        }
    }
};