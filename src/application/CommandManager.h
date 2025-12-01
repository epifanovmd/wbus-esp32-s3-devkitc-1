#pragma once
#include <Arduino.h>
#include <functional>
#include <queue>
#include <vector>
#include "./CommandReceiver.h"
#include "../common/Timer.h"
#include "../core/EventBus.h"
#include "../core/ConfigManager.h"
#include "../infrastructure/protocol/WBusCommand.h"
#include "../infrastructure/protocol/WBusErrorsDecoder.h"
#include "../interfaces/IBusManager.h"
#include "../domain/Events.h"

// Состояния обработки (аналог WBusQueueState из оригинала)
enum class ProcessingState {
    IDLE,              // Ожидание команды
    SENDING,           // Команда отправлена, ждем ответ
    RETRY,             // Повторная отправка
    BREAK_SET,         // BREAK сигнал установлен
    BREAK_RESET        // BREAK сигнал сброшен
};

Timer queueTimer(100);
Timer timeoutTimer(2000, false);
Timer breakTimer(50, false);

struct Command {
    String data;
    std::function<void(String tx, String rx)> callback;
    bool loop = false; // Зацикленное выполнение (для периодических запросов)
    
    Command() : data(""), callback(nullptr) {}
    
    Command(const String& cmd, std::function<void(String, String)> cb = nullptr, bool lp = false)
        : data(cmd), callback(cb), loop(lp) {}
};

class CommandManager {
private:
    std::queue<Command> queue;

    EventBus& eventBus;
    const BusConfig& config;
    IBusManager& busManager;

    CommanReceiver& commanReceiver;

    WBusErrorsDecoder errorsDecoder;
    
    // Текущее состояние
    ProcessingState state = ProcessingState::IDLE;
    uint8_t currentRetries = 0;
    const uint8_t MAX_RETRIES = 5;

public:
    CommandManager(EventBus& bus, IBusManager& busMngr, CommanReceiver& receiver) 
        : eventBus(bus), config(ConfigManager::getInstance().getConfig().bus)
        , commanReceiver(receiver)
        , busManager(busMngr)
    {
        // Подписываемся на события получения ответов
        // eventBus.subscribe(EventType::COMMAND_RECEIVED,
        //     [this](const Event& event) {
        //         handleEventResponse(event);
        //     });
    }
    
    // =========================================================================
    // ОСНОВНЫЕ МЕТОДЫ ДОБАВЛЕНИЯ КОМАНД
    // =========================================================================
    
    // Добавить команду в конец очереди
    bool addCommand(const String& command, std::function<void(String, String)> callback = nullptr, bool loop = false) {
        if (queue.size() >= 30) {
            Serial.println("❌ Очередь переполнена");
            return false;
        }

        if (containsCommand(command)) {
            return false;
        }
        
        queue.push(Command(command, callback, loop));
        
        return true;
    }
    
    // =========================================================================
    // УПРАВЛЕНИЕ ОЧЕРЕДЬЮ (аналоги оригинальных методов)
    // =========================================================================
    
    void process() {
        switch (state) {
            case ProcessingState::IDLE:
                if (!queue.empty() && queueTimer.isReady()) {
                    _sendCurrentCommand();
                }
                break;
                
            case ProcessingState::SENDING:
                if (commanReceiver.isRxReceived())
                {
                    // ✅ Ответ получен
                    _completeCurrentCommand(commanReceiver.getRxData(), true);
                }
                else if (timeoutTimer.isReady())
                {
                    // ⏰ Таймаут
                    _handleTimeout();
                }
                break;
                
            case ProcessingState::RETRY:
                if (breakTimer.isReady()) {
                    _sendCurrentCommand();
                }
                break;
                
            case ProcessingState::BREAK_SET:
                // BREAK set - удерживаем линию в LOW 50ms
                busManager.sendBreakSignal(true);
                breakTimer.reset();
                state = ProcessingState::BREAK_RESET;
                break;
                
            case ProcessingState::BREAK_RESET:
                if (breakTimer.isReady()) {
                    // BREAK reset - отпускаем линию и ждем 50ms
                    busManager.sendBreakSignal(false);
                    breakTimer.reset();
                    state = ProcessingState::RETRY;
                }
                break;
        }
    }
    
    // =========================================================================
    // МЕТОДЫ УПРАВЛЕНИЯ (аналоги оригинальных)
    // =========================================================================
    
    bool removeCommand(const String& command) {
        std::queue<Command> tempQueue;
        bool found = false;
        
        while (!queue.empty()) {
            Command cmd = queue.front();
            queue.pop();
            
            if (cmd.data == command) {
                found = true;
                Serial.println("🗑️  Команда удалена: " + command);
            } else {
                tempQueue.push(cmd);
            }
        }
        
        queue = std::move(tempQueue);
        return found;
    }
    
    bool containsCommand(const String& command) {
        std::queue<Command> tempQueue = queue;
        
        while (!tempQueue.empty()) {
            if (tempQueue.front().data == command) {
                return true;
            }
            tempQueue.pop();
        }
        return false;
    }
    
    void clear() {
        while (!queue.empty()) {
            queue.pop();
        }
        state = ProcessingState::IDLE;
        currentRetries = 0;
        
        Serial.println();
        Serial.println("🧹 Очередь очищена");
    }
    
    void setInterval(unsigned long interval) {
        queueTimer = interval;
    }
    
    void setTimeout(unsigned long timeout) {
        timeoutTimer = timeout;
        Serial.println();
        Serial.println("⏰ Таймаут установлен: " + String(timeout) + "мс");
    }
    
    // =========================================================================
    // СЛУЖЕБНЫЕ МЕТОДЫ
    // =========================================================================
    
    bool isEmpty() const {
        return queue.empty() && state == ProcessingState::IDLE;
    }
    
    size_t getPendingCount() const {
        return queue.size() + (state != ProcessingState::IDLE ? 1 : 0);
    }
    
    String getCurrentTx() const {
        return commanReceiver.getCurrentTx();
    }
    
    bool isWaitingForResponse() const {
        return state == ProcessingState::SENDING;
    }
    
    void printQueue() {
        Serial.println();
        Serial.println("📋 Содержимое очереди:");
        if (queue.empty() && state == ProcessingState::IDLE) {
            Serial.println("   (пусто)");
            return;
        }
        
        // Показываем команды в очереди
        std::queue<Command> tempQueue = queue;
        int index = 0;
        
        while (!tempQueue.empty()) {
            Command cmd = tempQueue.front();
            Serial.print("   ");
            Serial.print(index);
            Serial.print(": ");
            Serial.print(cmd.data);
            Serial.print(cmd.callback ? " [с колбэком]" : " [без колбэка]");
            if (cmd.loop) Serial.print(" [зациклена]");
            Serial.println();
            
            tempQueue.pop();
            index++;
        }
    }

private:
    // =========================================================================
    // ПРИВАТНЫЕ МЕТОДЫ ОБРАБОТКИ (аналоги оригинальных _методов)
    // =========================================================================
    
    void _sendCurrentCommand() {
        if (queue.empty()) return;
        
        Command command = queue.front();

        WBusCommand wBusCommand(command.data);
        
        // Валидация пакета (как в оригинале)
        // WBusPacket packet = WBusProtocol::parseHexStringToPacket(command.data);
        if (!wBusCommand.isValid()) {
            _completeCurrentCommand("", false);
            return;
        }
        
        if (busManager.sendCommand(wBusCommand.data, wBusCommand.byteCount)) {
            state = ProcessingState::SENDING;
            timeoutTimer.reset();
            eventBus.publish(EventType::COMMAND_SENT, command.data);
        } else {
            Serial.println();
            Serial.println("❌ Ошибка отправки команды: " + command.data);
            _completeCurrentCommand("", false);
        }
    }
    
    void _completeCurrentCommand(const String& response, bool success) {
        Command command = queue.front();
        queue.pop();

        // Вызываем колбэк если есть
        if (command.callback) {
            command.callback(command.data, success ? response : "");
        }
        
        // Если команда зациклена и успешно выполнена - добавляем обратно
        if (success && command.loop && !response.isEmpty()) {
            queue.push(command);
        }

        if (success) {
          eventBus.publish<CommandReceivedEvent>(EventType::COMMAND_RECEIVED, { command.data, response });
        } else {
            Serial.println();
            Serial.println("❌ Ошибка выполнения: " + command.data);
            eventBus.publish(EventType::COMMAND_SENT_ERRROR, command.data);
        }
        
        // Сброс состояния
        state = ProcessingState::IDLE;
        currentRetries = 0;
    }
    
    void _handleTimeout() {
        currentRetries++;
        Command command = queue.front();

        if (currentRetries > MAX_RETRIES) {
            _completeCurrentCommand("", false);
            clear();
        } else {
            eventBus.publish<ConnectionTimeoutEvent>(EventType::COMMAND_SENT_TIMEOUT, { currentRetries, command.data });
            Serial.println();
            Serial.println("🔄 Повторная отправка " + String(currentRetries) + "/" + String(MAX_RETRIES) + ": " + command.data);
            
            // Как в оригинале - BREAK сигнал перед повторной отправкой
            state = ProcessingState::BREAK_SET;
        }
    }
    
    void _processNakResponse(const String& response) {
        String cleanResponse = response;
        cleanResponse.replace(" ", "");

        if (cleanResponse.length() >= 10) {
            uint8_t failedCommand = Utils::hexStringToByte(cleanResponse.substring(8, 10));
            uint8_t errorCode = Utils::hexStringToByte(cleanResponse.substring(10, 12));

            Serial.println();
            Serial.println("❌ NAK получен:");
            Serial.println("   Невыполненная команда: 0x" + String(failedCommand, HEX));
            Serial.println("   Код ошибки: 0x" + String(errorCode, HEX));
            
            // Декодируем причину ошибки
            errorsDecoder.decodeNakError(failedCommand, errorCode);
        }
    }
};