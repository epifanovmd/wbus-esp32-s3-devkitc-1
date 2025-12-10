// src/domain/Events.h
#pragma once
#include "Entities.h"


struct AppConfigUpdateEvent
{
    AppConfig config;
};

struct ConnectionTimeoutEvent
{
    int retrie;
    String tx;

    String toJson() const
    {
        String json = "{";
        json += "\"retrie\":" + String(retrie) + ",";
        json += "\"tx\":\"" + tx + "\"";
        json += "}";
        return json;
    }
};

struct CommandReceivedEvent
{
    String tx;
    String rx;

    String toJson() const
    {
        String json = "{";
        json += "\"tx\":\"" + tx + "\",";
        json += "\"rx\":\"" + rx + "\"";
        json += "}";
        return json;
    }
};

struct HeaterStateChangedEvent
{
    WebastoState oldState;
    WebastoState newState;

    String toJson() const
    {
        String json = "{";
        json += "\"oldState\":\"" + HeaterStatus::getStateName(oldState) + "\",";
        json += "\"newState\":\"" + HeaterStatus::getStateName(newState) + "\"";
        json += "}";
        return json;
    }
};

struct ConnectionStateChangedEvent
{
    ConnectionState oldState;
    ConnectionState newState;

    String toJson() const
    {
        String json = "{";
        json += "\"oldState\":\"" + HeaterStatus::getConnectionName(oldState) + "\",";
        json += "\"newState\":\"" + HeaterStatus::getConnectionName(newState) + "\"";
        json += "}";
        return json;
    }
};

struct NakResponseEvent
{
    String tx;
    String commandName;
    uint8_t errorCode;

    NakResponseEvent(const String &cmdTx, const String cmddName, uint8_t errCode = 0)
        : tx(cmdTx), commandName(cmddName), errorCode(errCode) {}

    String toJson() const
    {
        String json = "{";
        json += "\"tx\":\"" + tx + "\",";
        json += "\"commandName\":\"" + commandName + "\",";
        json += "\"errorCode\":\"0x" + String(errorCode, HEX) + "\",";
        json += "\"errorDescription\":\"" + getErrorDescription(errorCode) + "\"";
        json += "}";
        return json;
    }

private:
    String getErrorDescription(uint8_t errorCode) const
    {
        switch (errorCode)
        {
        case 0x33:
            return "Невозможно выполнить в текущем состоянии";
        case 0x22:
            return "Неправильные параметры команды";
        case 0x11:
            return "Команда не поддерживается";
        case 0x44:
            return "Аппаратная ошибка";
        case 0x55:
            return "Температура вне диапазона";
        default:
            return "Неизвестная ошибка (0x" + String(errorCode, HEX) + ")";
        }
    }
};