// src/infrastructure/network/WiFiManager.h
#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <DNSServer.h>
#include <ArduinoJson.h>
#include "../../core/ConfigManager.h"
#include "../../core/EventBus.h"
#include "../../domain/Events.h"

// Состояния WiFi
enum class WiFiState
{
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    AP_MODE,
    SMART_CONFIG
};

class WiFiManager
{
private:
    ConfigManager &configManager;
    EventBus &eventBus;

    // DNS сервер для captive portal
    DNSServer dnsServer;

    // Таймеры
    unsigned long lastConnectionAttempt = 0;
    unsigned long connectionStartTime = 0;
    const unsigned long CONNECTION_TIMEOUT = 30000; // 30 секунд

    // Состояние
    WiFiState currentState = WiFiState::DISCONNECTED;
    String currentSsid = "";
    String currentIp = "";

    // mDNS
    bool mdnsStarted = false;

public:
    WiFiManager(ConfigManager &config, EventBus &bus)
        : configManager(config), eventBus(bus) {}

    // =========================================================================
    // ОСНОВНЫЕ МЕТОДЫ
    // =========================================================================

    bool initialize()
    {
        Serial.println("\n📡 WiFi Manager Initializing...");

        auto &netConfig = configManager.getConfig().network;

        // Настраиваем hostname
        WiFi.setHostname(netConfig.hostname.c_str());

        // Определяем режим работы
        switch (netConfig.mode)
        {
        case NetworkConfig::WifiMode::AP:
            return startAPMode();

        case NetworkConfig::WifiMode::AP_STA:
            return startAPSTAMode();

        default:
            return startAPMode();
        }
    }

    void process()
    {
        // Мониторинг подключения в STA режиме
        if (WiFi.getMode() & WIFI_STA)
        {
            monitorConnection();
        }
    }

    // =========================================================================
    // ПУБЛИЧНЫЕ МЕТОДЫ
    // =========================================================================

    WiFiState getState() const
    {
        return currentState;
    }

    String getIP() const
    {
        return currentIp;
    }

    String getSSID() const
    {
        return currentSsid;
    }

    bool isConnected() const
    {
        return WiFi.status() == WL_CONNECTED;
    }

    String getAccessURL() const
    {
        auto &netConfig = configManager.getConfig().network;

        if (WiFi.getMode() & WIFI_STA && WiFi.status() == WL_CONNECTED)
        {
            return "http://" + WiFi.localIP().toString() + ":" + String(netConfig.port);
        }
        else if (WiFi.getMode() & WIFI_AP)
        {
            return "http://" + WiFi.softAPIP().toString() + ":" + String(netConfig.port);
        }
        return "";
    }

    String getMDNSURL() const
    {
        auto &netConfig = configManager.getConfig().network;
        return "http://" + netConfig.hostname + ".local:" + String(netConfig.port);
    }

    // =========================================================================
    // УПРАВЛЕНИЕ ПОДКЛЮЧЕНИЕМ
    // =========================================================================

    bool disconnectWiFi()
    {
        Serial.println("🔌 Disconnecting WiFi...");
        WiFi.disconnect(true);
        setState(WiFiState::DISCONNECTED, "", "", "Disconnected");
        return true;
    }

    bool restartWiFi()
    {
        Serial.println("🔄 Restarting WiFi...");

        // Останавливаем mDNS если запущен
        if (mdnsStarted)
        {
            MDNS.end();
            mdnsStarted = false;
        }

        WiFi.disconnect(true);
        delay(1000);
        return initialize();
    }

    // =========================================================================
    // mDNS
    // =========================================================================

    bool startMDNS()
    {
        auto &netConfig = configManager.getConfig().network;

        if (mdnsStarted)
        {
            MDNS.end();
        }

        if (!MDNS.begin(netConfig.hostname.c_str()))
        {
            Serial.println("❌ Error starting mDNS responder!");
            mdnsStarted = false;
            return false;
        }

        mdnsStarted = true;

        // Добавляем сервисы
        MDNS.addService("http", "tcp", netConfig.port);
        MDNS.addService("webasto", "tcp", netConfig.port);

        Serial.println("✅ mDNS started: " + netConfig.hostname + ".local");
        return true;
    }

    void stopMDNS()
    {
        if (mdnsStarted)
        {
            MDNS.end();
            mdnsStarted = false;
            Serial.println("mDNS stopped");
        }
    }

    bool isMDNSStarted() const
    {
        return mdnsStarted;
    }

    // =========================================================================
    // ПРИВАТНЫЕ МЕТОДЫ
    // =========================================================================

private:
    bool startSTAMode()
    {
        auto &netConfig = configManager.getConfig().network;

        // Проверяем наличие учетных данных
        if (netConfig.staSsid.isEmpty())
        {
            Serial.println("⚠️  No WiFi credentials");
            return false;
        }

        Serial.println("📡 Starting STA Mode");
        Serial.println("  SSID: " + netConfig.staSsid);

        WiFi.mode(WIFI_STA);

        // Оптимизации WiFi
        WiFi.setSleep(false);

        setState(WiFiState::CONNECTING, netConfig.staSsid, "", "Connecting...");

        connectionStartTime = millis();

        // Начинаем подключение (асинхронно)
        WiFi.begin(netConfig.staSsid.c_str(), netConfig.staPassword.c_str());

        return true;
    }

    bool startAPMode()
    {
        auto &netConfig = configManager.getConfig().network;

        Serial.println("📡 Starting AP Mode");
        Serial.println("  SSID: " + netConfig.apSsid);

        WiFi.mode(WIFI_AP);

        WiFi.softAPConfig(
            IPAddress(192, 168, 4, 1),
            IPAddress(192, 168, 4, 1),
            IPAddress(255, 255, 255, 0));

        if (!WiFi.softAP(netConfig.apSsid.c_str(), netConfig.apPassword.c_str()))
        {
            Serial.println("❌ Failed to start AP");
            return false;
        }

        // Запускаем mDNS
        startMDNS();

        setState(WiFiState::AP_MODE, netConfig.apSsid, WiFi.softAPIP().toString(), "Access Point started");

        return true;
    }

    bool startAPSTAMode()
    {
        auto &netConfig = configManager.getConfig().network;

        Serial.println("📡 Starting AP+STA Mode");

        WiFi.mode(WIFI_AP_STA);

        // Запускаем AP
        if (!netConfig.apSsid.isEmpty())
        {
            WiFi.softAP(netConfig.apSsid.c_str(), netConfig.apPassword.c_str());
        }

        // Пытаемся подключиться к STA
        if (!netConfig.staSsid.isEmpty())
        {
            WiFi.begin(netConfig.staSsid.c_str(), netConfig.staPassword.c_str());
            setState(WiFiState::CONNECTING, netConfig.staSsid, "", "Connecting...");
            connectionStartTime = millis();
        }

        // Запускаем mDNS
        startMDNS();

        setState(WiFiState::AP_MODE, netConfig.apSsid, WiFi.softAPIP().toString(), "AP+STA mode started");

        return true;
    }

    void monitorConnection()
    {
        static wl_status_t lastStatus = WL_IDLE_STATUS;
        wl_status_t currentStatus = WiFi.status();

        // Если статус изменился
        if (currentStatus != lastStatus)
        {
            lastStatus = currentStatus;

            switch (currentStatus)
            {
            case WL_CONNECTED:
                onConnected();
                break;

            case WL_DISCONNECTED:
                onDisconnected();
                break;

            case WL_CONNECT_FAILED:
                onConnectionFailed();
                break;

            case WL_NO_SSID_AVAIL:
                onNoSSIDAvailable();
                break;

            default:
                break;
            }
        }

        // Проверка таймаута подключения
        if (currentState == WiFiState::CONNECTING &&
            millis() - connectionStartTime > CONNECTION_TIMEOUT)
        {
            Serial.println("⏰ Connection timeout");
            onConnectionFailed();
        }

        // Автоматическое переподключение
        auto &netConfig = configManager.getConfig().network;
        if (currentState == WiFiState::DISCONNECTED && millis() - lastConnectionAttempt > netConfig.reconnectInterval)
        {

            Serial.println("🔄 Attempting to reconnect...");
            lastConnectionAttempt = millis();

            if (!netConfig.staSsid.isEmpty())
            {
                WiFi.reconnect();
                setState(WiFiState::CONNECTING, netConfig.staSsid, "", "Reconnecting...");
            }
        }
    }

    void onConnected()
    {
        currentIp = WiFi.localIP().toString();
        currentSsid = WiFi.SSID();

        // Запускаем mDNS
        startMDNS();

        setState(WiFiState::CONNECTED, currentSsid, currentIp,
                 "Successfully connected");

        Serial.println("✅ WiFi Connected!");
        Serial.println("  IP: " + currentIp);
        Serial.println("  RSSI: " + String(WiFi.RSSI()) + " dBm");
        Serial.println("  Gateway: " + WiFi.gatewayIP().toString());
        Serial.println("  mDNS: " + getMDNSURL());
    }

    void onDisconnected()
    {
        setState(WiFiState::DISCONNECTED, "", "", "Disconnected from WiFi");
        Serial.println("🔌 WiFi Disconnected");

        // Если мы в STA режиме и есть учетные данные, пытаемся переподключиться
        if (WiFi.getMode() & WIFI_STA)
        {
            auto &netConfig = configManager.getConfig().network;
            if (!netConfig.staSsid.isEmpty())
            {
                connectionStartTime = millis();
                setState(WiFiState::CONNECTING, netConfig.staSsid, "", "Reconnecting...");
            }
        }
    }

    void onConnectionFailed()
    {
        setState(WiFiState::DISCONNECTED, "", "", "Connection failed");
        Serial.println("❌ WiFi Connection Failed");
    }

    void onNoSSIDAvailable()
    {
        setState(WiFiState::DISCONNECTED, "", "", "SSID not available");
        Serial.println("❌ SSID not available");
    }

    void setState(WiFiState state, const String &ssid, const String &ip, const String &message)
    {

        // Сохраняем предыдущее состояние
        WiFiState oldState = currentState;

        // Обновляем текущее состояние
        currentState = state;
        currentSsid = ssid;
        currentIp = ip;

        // Логируем изменение состояния
        if (oldState != state)
        {
            Serial.println("📶 WiFi State: " + getStateString(oldState) + " → " + getStateString(state));
        }
    }

    String getStateString(WiFiState state) const
    {
        switch (state)
        {
        case WiFiState::DISCONNECTED:
            return "DISCONNECTED";
        case WiFiState::CONNECTING:
            return "CONNECTING";
        case WiFiState::CONNECTED:
            return "CONNECTED";
        case WiFiState::AP_MODE:
            return "AP_MODE";
        default:
            return "UNKNOWN";
        }
    }
};