#pragma once
#include <Arduino.h>
#include <functional>
#include <deque>
#include <vector>
#include "./CommandReceiver.h"
#include "../common/Timer.h"
#include "../core/EventBus.h"
#include "../core/ConfigManager.h"
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
    std::deque<Command> priorityDeque; // Было priorityQueue
    std::deque<Command> normalDeque;   // Было normalQueue

    ConfigManager &configManager;

    EventBus &eventBus;
    IBusManager &busManager;
    CommandReceiver &commandReceiver;

    WBusErrorsDecoder errorsDecoder;
    PacketParser packetParser;

    Command processingCommand;
    ProcessingState state = ProcessingState::IDLE;
    uint8_t currentRetries = 0;

    Timer queueTimer;
    Timer timeoutTimer;
    Timer breakTimer;

public:
    CommandManager(ConfigManager &configMngr, EventBus &bus, IBusManager &busMngr, CommandReceiver &receiver)
        : configManager(configMngr),
          eventBus(bus),
          commandReceiver(receiver),
          busManager(busMngr),
          queueTimer(configMngr.getConfig().bus.queueInterval),
          timeoutTimer(configMngr.getConfig().bus.commandTimeout, false),
          breakTimer(configMngr.getConfig().bus.breakSignalDuration, false)
    {
        eventBus.subscribe(EventType::APP_CONFIG_UPDATE,
                           [this](const Event &event)
                           {
                               const auto &configEvent = static_cast<
                                   const TypedEvent<AppConfigUpdateEvent> &>(event);

                               setInterval(configEvent.data.config.bus.queueInterval);
                               setTimeout(configEvent.data.config.bus.commandTimeout);
                               setBreakTimeout(configEvent.data.config.bus.breakSignalDuration);
                           });
    }

    void initialize()
    {
        setInterval(configManager.getConfig().bus.queueInterval);
        setTimeout(configManager.getConfig().bus.commandTimeout);
        setBreakTimeout(configManager.getConfig().bus.breakSignalDuration);
    }

    bool addCommand(const String &command, bool loop = false, std::function<void(String, String)> callback = nullptr)
    {
        if (getTotalQueueSize() >= configManager.getConfig().bus.maxQueueSize || containsCommand(command))
            return false;

        normalDeque.push_back(Command(command, callback, loop));
        return true;
    }

    bool addPriorityCommand(const String &command, bool loop = false, std::function<void(String, String)> callback = nullptr)
    {
        if (getTotalQueueSize() >= configManager.getConfig().bus.maxPriorityQueueSize || containsPriorityCommand(command))
            return false;

        priorityDeque.push_back(Command(command, callback, loop));
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
            if (commandReceiver.isRxReceived())
            {
                // ✅ Ответ получен
                complete(commandReceiver.getRxData());
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
        for (const auto &cmd : normalDeque)
        {
            if (cmd.data == command)
                return true;
        }
        return false;
    }

    bool containsPriorityCommand(const String &command)
    {
        for (const auto &cmd : priorityDeque)
        {
            if (cmd.data == command)
                return true;
        }
        return false;
    }

    void clear()
    {
        priorityDeque.clear();
        normalDeque.clear();

        state = ProcessingState::IDLE;
        currentRetries = 0;
        processingCommand = Command();

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

    void setBreakTimeout(unsigned long timeout)
    {
        breakTimer.setInterval(timeout);
    }
    bool isEmpty() const
    {
        return isQueueEmpty() && state == ProcessingState::IDLE;
    }

    size_t getTotalQueueSize() const {
        return priorityDeque.size() + normalDeque.size();
    }

private:
    bool isQueueEmpty() const {
        return priorityDeque.empty() && normalDeque.empty();
    }

    Command getNextCommand()
    {
        if (!priorityDeque.empty())
        {
            Command cmd = priorityDeque.front();
            priorityDeque.pop_front();
            return cmd;
        }
        else if (!normalDeque.empty())
        {
            Command cmd = normalDeque.front();
            normalDeque.pop_front();
            return cmd;
        }
        return Command();
    }

    void sendCurrentCommand()
    {
        if (packetParser.parseFromString(processingCommand.data))
        {
            if (busManager.sendCommand(packetParser.getData(), packetParser.getByteCounts()))
            {
                timeoutTimer.reset();
                state = ProcessingState::SENDING;
            }
            else
            {
                eventBus.publish(EventType::COMMAND_SENT_ERRROR, processingCommand.data);
                clear();
            }
        }
    }

    void complete(const String &response)
    {
        if (processingCommand.callback)
        {
            processingCommand.callback(processingCommand.data, response);
        }

        else if (!Utils::isNakPacket(response))
        {
            if (processingCommand.loop)
            {
                normalDeque.push_back(Command(processingCommand.data, processingCommand.callback, processingCommand.loop));
            }
        }

        state = ProcessingState::IDLE;
        currentRetries = 0;
        processingCommand = Command();
    }

    void handleTimeout()
    {
        currentRetries++;

        uint8_t maxRetries = configManager.getConfig().bus.maxRetries;

        if (currentRetries > maxRetries)
        {
            eventBus.publish(EventType::COMMAND_SENT_ERRROR, processingCommand.data);
            clear();
        }
        else
        {
            eventBus.publish<ConnectionTimeoutEvent>(EventType::COMMAND_SENT_TIMEOUT, {currentRetries, processingCommand.data});
            Serial.println("🔄 Повторная отправка " + String(currentRetries) + "/" + String(maxRetries) + ": " + processingCommand.data);

            state = ProcessingState::BREAK_SET;
        }
    }
};