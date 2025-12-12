// src/infrastructure/network/WebSocketSubscriptionManager.h
#pragma once
#include <Arduino.h>
#include <map>
#include <set>
#include "./core/EventBus.h"

class WebSocketSubscriptionManager
{
private:
    // Клиент -> набор подписок
    std::map<uint32_t, std::set<EventType>> clientSubscriptions;

    // Событие -> набор клиентов
    std::map<EventType, std::set<uint32_t>> eventSubscribers;

public:
    // Подписать клиента на событие
    void subscribe(uint32_t clientId, EventType eventType)
    {
        clientSubscriptions[clientId].insert(eventType);
        eventSubscribers[eventType].insert(clientId);
    }

    // Отписать клиента от события
    void unsubscribe(uint32_t clientId, EventType eventType)
    {
        clientSubscriptions[clientId].erase(eventType);
        eventSubscribers[eventType].erase(clientId);

        // Удалить пустые записи
        if (clientSubscriptions[clientId].empty())
        {
            clientSubscriptions.erase(clientId);
        }
    }

    // Получить все подписки клиента
    std::set<EventType> getClientSubscriptions(uint32_t clientId) const
    {
        auto it = clientSubscriptions.find(clientId);
        if (it != clientSubscriptions.end())
        {
            return it->second;
        }
        return {};
    }

    // Получить всех подписчиков события
    std::set<uint32_t> getEventSubscribers(EventType eventType) const
    {
        auto it = eventSubscribers.find(eventType);
        if (it != eventSubscribers.end())
        {
            return it->second;
        }
        return {};
    }

    // Отписать клиента от всех событий (при отключении)
    void unsubscribeAll(uint32_t clientId)
    {
        auto it = clientSubscriptions.find(clientId);
        if (it != clientSubscriptions.end())
        {
            for (auto eventType : it->second)
            {
                eventSubscribers[eventType].erase(clientId);

                // Удалить пустые записи событий
                if (eventSubscribers[eventType].empty())
                {
                    eventSubscribers.erase(eventType);
                }
            }
            clientSubscriptions.erase(clientId);
        }
    }

    // Проверить, подписан ли клиент на событие
    bool isSubscribed(uint32_t clientId, EventType eventType) const
    {
        auto it = clientSubscriptions.find(clientId);
        if (it != clientSubscriptions.end())
        {
            return it->second.find(eventType) != it->second.end();
        }
        return false;
    }
};