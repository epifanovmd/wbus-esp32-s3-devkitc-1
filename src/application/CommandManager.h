#pragma once
#include <Arduino.h>
#include <functional>
#include <queue>
#include <vector>
#include "./CommanReceiver.h"
#include "../common/Timer.h"
#include "../core/EventBus.h"
#include "../core/ConfigManager.h"
#include "../infrastructure/protocol/WBusProtocol.h"
#include "../infrastructure/protocol/WBusErrorsDecoder.h"
#include "../interfaces/IBusManager.h"
#include "../domain/Events.h"

// Приоритеты команд (как в оригинальном коде)
enum class CommandPriority {
    PRIORITY_HIGH = 1,    // Для немедленного выполнения
    PRIORITY_NORMAL = 2,  // Стандартный приоритет  
    PRIORITY_LOW = 3      // Фоновые задачи
};

// Состояния обработки (аналог WBusQueueState из оригинала)
enum class ProcessingState {
    IDLE,              // Ожидание команды
    SENDING,           // Команда отправлена, ждем ответ
    RETRY,             // Повторная отправка
    BREAK_SET,         // BREAK сигнал установлен
    BREAK_RESET        // BREAK сигнал сброшен
};

Timer queueTimer(150);
Timer timeoutTimer(2000, false);
Timer breakTimer(50, false);

struct Command {
    String data;
    CommandPriority priority;
    std::function<void(String tx, String rx)> callback;
    bool loop = false; // Зацикленное выполнение (для периодических запросов)
    
    Command() : data(""), priority(CommandPriority::PRIORITY_NORMAL), 
                callback(nullptr) {}
    
    Command(const String& cmd, CommandPriority prio = CommandPriority::PRIORITY_NORMAL, 
            std::function<void(String, String)> cb = nullptr, bool lp = false)
        : data(cmd), priority(prio), callback(cb), loop(lp) {}
    
    // Для priority_queue - меньший приоритетный номер = выше приоритет
    bool operator<(const Command& other) const {
        return static_cast<int>(priority) > static_cast<int>(other.priority);
    }
};

class CommandManager {
private:
    std::priority_queue<Command> queue;

    EventBus& eventBus;
    const BusConfig& config;
    IBusManager& busManager;

    CommanReceiver& commanReceiver;

    WBusErrorsDecoder errorsDecoder;
    
    // Текущее состояние
    ProcessingState state = ProcessingState::IDLE;
    Command currentCommand;
    uint8_t currentRetries = 0;
    const uint8_t MAX_RETRIES = 5;

public:
    CommandManager(EventBus& bus, IBusManager& busMngr, CommanReceiver& receiver) 
        : eventBus(bus), config(ConfigManager::getInstance().getConfig().bus)
        , commanReceiver(receiver)
        , busManager(busMngr)
    {
        // Подписываемся на события получения ответов
        eventBus.subscribe(EventType::COMMAND_RECEIVED,
            [this](const Event& event) {
                handleEventResponse(event);
            });
    }
    
    // =========================================================================
    // ОСНОВНЫЕ МЕТОДЫ ДОБАВЛЕНИЯ КОМАНД (аналоги оригинальных QueueMap)
    // =========================================================================
    
    // Добавить команду в конец очереди (аналог QueueMap::add)
    bool addCommand(const String& command, std::function<void(String, String)> callback = nullptr, bool loop = false) {
        if (queue.size() >= 30) {
            Serial.println("❌ Очередь переполнена");
            return false;
        }
        
        // Проверяем нет ли уже такой команды в очереди (как в оригинале)
        if (containsCommand(command)) {
            return false;
        }
        
        queue.push(Command(command, CommandPriority::PRIORITY_NORMAL, callback, loop));
        
        Serial.println("📋 Команда добавлена в очередь: " + command);
        
        return true;
    }
    
    // Добавить команду в начало очереди (аналог QueueMap::addPriority)
    bool addPriorityCommand(const String& command, std::function<void(String, String)> callback = nullptr, bool loop = false) {
        if (queue.size() >= 30) {
            Serial.println("❌ Очередь переполнена");
            return false;
        }
        
        if (containsCommand(command)) {
            return false;
        }
        
        // Создаем временную очередь для перестановки приоритетов
        std::priority_queue<Command> tempQueue;
        tempQueue.push(Command(command, CommandPriority::PRIORITY_HIGH, callback, loop));
        
        // Переносим все существующие команды
        while (!queue.empty()) {
            tempQueue.push(queue.top());
            queue.pop();
        }
        
        queue = std::move(tempQueue);
        
        Serial.println("🚀 Приоритетная команда добавлена: " + command);
        
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
                    Serial.println("isRxReceived");
                    // ✅ Ответ получен
                    _completeCurrentCommand(commanReceiver.getRxData(), true);
                }
                // else if (timeoutTimer.isReady())
                // {
                //     // ⏰ Таймаут
                //     _handleTimeout();
                // }
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
    
    void handleEventResponse(const Event& event) {
        const auto& responseEvent = static_cast<const TypedEvent<CommandReceivedEvent>&>(event);

        if (state == ProcessingState::SENDING) {
            _completeCurrentCommand(responseEvent.data.response, responseEvent.data.success);
            
            // Обработка NAK ответов
            if (errorsDecoder.isNakResponse(responseEvent.data.response)) {
                _processNakResponse(responseEvent.data.response);
            }
        }
    }
    
    // =========================================================================
    // МЕТОДЫ УПРАВЛЕНИЯ (аналоги оригинальных)
    // =========================================================================
    
    bool removeCommand(const String& command) {
        std::priority_queue<Command> tempQueue;
        bool found = false;
        
        while (!queue.empty()) {
            Command cmd = queue.top();
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
        std::priority_queue<Command> tempQueue = queue;
        
        while (!tempQueue.empty()) {
            if (tempQueue.top().data == command) {
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
        
        Serial.println("🧹 Очередь очищена");
    }
    
    void setInterval(unsigned long interval) {
        queueTimer = interval;
    }
    
    void setTimeout(unsigned long timeout) {
        timeoutTimer = timeout;
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
        Serial.println("📋 Содержимое очереди:");
        if (queue.empty() && state == ProcessingState::IDLE) {
            Serial.println("   (пусто)");
            return;
        }
        
        // Показываем текущую обрабатываемую команду
        if (state != ProcessingState::IDLE) {
            Serial.println("   [ТЕКУЩАЯ] " + currentCommand.data + 
                          " [попытка " + String(currentRetries) + "]");
        }
        
        // Показываем команды в очереди
        std::priority_queue<Command> tempQueue = queue;
        int index = 0;
        
        while (!tempQueue.empty()) {
            Command cmd = tempQueue.top();
            Serial.print("   ");
            Serial.print(index);
            Serial.print(": ");
            Serial.print(cmd.data);
            Serial.print(cmd.callback ? " [с колбэком]" : " [без колбэка]");
            if (cmd.loop) Serial.print(" [зациклена]");
            if (cmd.priority == CommandPriority::PRIORITY_HIGH) Serial.print(" [ВЫСОКИЙ ПРИОРИТЕТ]");
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
        
        currentCommand = queue.top();
        queue.pop();
        
        // Валидация пакета (как в оригинале)
        WBusPacket packet = WBusProtocol::parseHexStringToPacket(currentCommand.data);
        if (!WBusProtocol::validateWbusPacket(packet)) {
            Serial.println("❌ Неверный пакет: " + currentCommand.data);
            _completeCurrentCommand("", false);
            return;
        }
        
        if (busManager.sendCommand(currentCommand.data)) {
            state = ProcessingState::SENDING;
            timeoutTimer.reset();
            currentRetries = 0;

        } else {
            Serial.println("❌ Ошибка отправки команды: " + currentCommand.data);
            _completeCurrentCommand("", false);
        }
    }
    
    void _completeCurrentCommand(const String& response, bool success) {
        // Вызываем колбэк если есть
        if (currentCommand.callback) {
            currentCommand.callback(currentCommand.data, success ? response : "");
        }
        
        // Если команда зациклена и успешно выполнена - добавляем обратно
        if (success && currentCommand.loop && !response.isEmpty()) {
            queue.push(Command(currentCommand.data, currentCommand.priority, currentCommand.callback, true));
        }
        
        // Событие о получении ответа
        eventBus.publish<CommandReceivedEvent>(EventType::COMMAND_RECEIVED, {currentCommand.data, response, success});

        if (success) {
            Serial.println("✅ Ответ получен для: " + currentCommand.data);
            if (!response.isEmpty()) {
                Serial.println("📨 RX: " + response);
            }
        } else {
            Serial.println("❌ Ошибка выполнения: " + currentCommand.data);
        }

        
        // Сброс состояния
        state = ProcessingState::IDLE;
        currentRetries = 0;
        
        // Если очередь пуста - сообщаем
        if (queue.empty()) {
            Serial.println("ℹ️  Очередь команд пуста");
        }
    }
    
    void _handleTimeout() {
        currentRetries++;
        
        if (currentRetries >= MAX_RETRIES) {
            Serial.println("❌ Таймаут после " + String(currentRetries) + " попыток: " + currentCommand.data);
            _completeCurrentCommand("", false);
        } else {
            Serial.println("🔄 Повторная отправка " + String(currentRetries) + "/" + String(MAX_RETRIES) + ": " + currentCommand.data);
            
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