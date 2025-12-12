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

class WiFiManager
{
private:
    ConfigManager &configManager;
    EventBus &eventBus;

    // DNS сервер для captive portal
    DNSServer dnsServer;

    // Таймеры
    unsigned long lastConnectionAttempt = 0;
    const unsigned long CONNECTION_TIMEOUT = 30000; // 30 секунд
    unsigned long connectionStartTime = 0;

    // mDNS
    bool mdnsStarted = false;

    // Флаги состояния
    bool connectionInProgress = false;
    bool isAPMode = false;
    bool isSTAMode = false;

public:
    WiFiManager(ConfigManager &config, EventBus &bus)
        : configManager(config), eventBus(bus)
    {
        // Настраиваем обработчики событий WiFi
        WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info)
                     { this->handleWiFiEvent(event, info); });
    }

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
        // Проверка таймаута подключения STA
        if (connectionInProgress &&
            millis() - connectionStartTime > CONNECTION_TIMEOUT)
        {
            Serial.println("⏰ WiFi connection timeout");
            connectionInProgress = false;
            WiFi.disconnect(false);
        }

        // Автоматическое переподключение STA
        auto &netConfig = configManager.getConfig().network;
        if (!isConnected() &&
            isSTAMode &&
            !connectionInProgress &&
            millis() - lastConnectionAttempt > netConfig.reconnectInterval &&
            !netConfig.staSsid.isEmpty())
        {
            Serial.println("🔄 Attempting to reconnect WiFi...");
            lastConnectionAttempt = millis();

            connectionInProgress = true;
            connectionStartTime = millis();

            WiFi.reconnect();
        }
    }

    // =========================================================================
    // ОБРАБОТЧИКИ СОБЫТИЙ WiFi
    // =========================================================================

private:
    void handleWiFiEvent(arduino_event_id_t event, arduino_event_info_t info)
    {
        switch (event)
        {
        // События подключения STA
        case ARDUINO_EVENT_WIFI_STA_START:
            Serial.println("📡 WiFi STA mode started");
            isSTAMode = true;
            connectionInProgress = true;
            connectionStartTime = millis();
            break;

        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.println("📡 Connected to SSID: " + String((char *)info.wifi_sta_connected.ssid));
            break;

        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            onWiFiConnected(
                IPAddress(info.got_ip.ip_info.ip.addr).toString(),
                IPAddress(info.got_ip.ip_info.gw.addr).toString());
            break;

        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            onWiFiDisconnected(info.wifi_sta_disconnected.reason);
            break;

        // События точки доступа AP
        case ARDUINO_EVENT_WIFI_AP_START:
            isAPMode = true;
            Serial.println("📡 WiFi AP mode started");
            Serial.println("  IP: " + WiFi.softAPIP().toString());
            startMDNS();
            break;

        case ARDUINO_EVENT_WIFI_AP_STOP:
            isAPMode = false;
            Serial.println("📡 WiFi AP mode stopped");
            break;

        // Общие события
        case ARDUINO_EVENT_WIFI_READY:
            Serial.println("📡 WiFi hardware ready");
            break;

        default:
            break;
        }
    }

    void onWiFiConnected(const String &ip, const String &gateway)
    {
        connectionInProgress = false;

        startMDNS();

        Serial.println("✅ WiFi Connected!");
        Serial.println("  IP: " + ip);
        Serial.println("  Gateway: " + gateway);
        Serial.println("  RSSI: " + String(WiFi.RSSI()) + " dBm");

        if (mdnsStarted)
        {
            auto &netConfig = configManager.getConfig().network;
            Serial.println("  mDNS: " + netConfig.hostname + ".local");
        }
    }

    void onWiFiDisconnected(uint8_t reason)
    {
        connectionInProgress = false;

        Serial.println("🔌 WiFi Disconnected");
        Serial.println("  Reason: " + getDisconnectReason(reason));

        // mDNS работает только в AP режиме
        if (!isAPMode)
        {
            stopMDNS();
        }
    }

    String getDisconnectReason(uint8_t reason) const
    {
        // Краткие основные причины
        switch (reason)
        {
        case WIFI_REASON_AUTH_EXPIRE:
            return "Auth expired";
        case WIFI_REASON_NO_AP_FOUND:
            return "No AP found";
        case WIFI_REASON_AUTH_FAIL:
            return "Auth failed";
        case WIFI_REASON_ASSOC_FAIL:
            return "Association failed";
        case WIFI_REASON_HANDSHAKE_TIMEOUT:
            return "Handshake timeout";
        case WIFI_REASON_CONNECTION_FAIL:
            return "Connection failed";
        case WIFI_REASON_BEACON_TIMEOUT:
            return "Beacon timeout";
        default:
            return "Reason code: " + String(reason);
        }
    }

    // =========================================================================
    // ПУБЛИЧНЫЕ МЕТОДЫ
    // =========================================================================

public:
    String getIP() const
    {
        if (WiFi.getMode() & WIFI_STA && WiFi.status() == WL_CONNECTED)
        {
            return WiFi.localIP().toString();
        }
        else if (WiFi.getMode() & WIFI_AP)
        {
            return WiFi.softAPIP().toString();
        }
        return "";
    }

    String getSSID() const
    {
        if (WiFi.getMode() & WIFI_STA && WiFi.status() == WL_CONNECTED)
        {
            return WiFi.SSID();
        }
        else if (WiFi.getMode() & WIFI_AP)
        {
            auto &netConfig = configManager.getConfig().network;
            return netConfig.apSsid;
        }
        return "";
    }

    bool isConnected() const
    {
        return WiFi.status() == WL_CONNECTED;
    }

    bool isAccessPoint() const
    {
        return isAPMode;
    }

    String getAccessURL() const
    {
        auto &netConfig = configManager.getConfig().network;
        String ip = getIP();

        if (!ip.isEmpty())
        {
            return "http://" + ip + ":" + String(netConfig.port);
        }
        return "";
    }

    String getMDNSURL() const
    {
        if (!mdnsStarted)
            return "";

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
        connectionInProgress = false;
        return true;
    }

    bool restartWiFi()
    {
        Serial.println("🔄 Restarting WiFi...");

        stopMDNS();
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
            Serial.println("❌ Failed to start mDNS");
            mdnsStarted = false;
            return false;
        }

        mdnsStarted = true;

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
    // ПРИВАТНЫЕ МЕТОДЫ РЕЖИМОВ
    // =========================================================================

private:
    bool startSTAMode()
    {
        auto &netConfig = configManager.getConfig().network;

        if (netConfig.staSsid.isEmpty())
        {
            Serial.println("⚠️  No WiFi credentials for STA mode");
            return false;
        }

        Serial.println("📡 Starting WiFi STA mode");
        Serial.println("  SSID: " + netConfig.staSsid);

        WiFi.mode(WIFI_STA);
        WiFi.setSleep(false);

        connectionInProgress = true;
        connectionStartTime = millis();

        WiFi.begin(netConfig.staSsid.c_str(), netConfig.staPassword.c_str());

        return true;
    }

    bool startAPMode()
    {
        auto &netConfig = configManager.getConfig().network;

        Serial.println("📡 Starting WiFi AP mode");
        Serial.println("  SSID: " + netConfig.apSsid);

        WiFi.mode(WIFI_AP);

        WiFi.softAPConfig(
            IPAddress(192, 168, 4, 1),
            IPAddress(192, 168, 4, 1),
            IPAddress(255, 255, 255, 0));

        return WiFi.softAP(netConfig.apSsid.c_str(), netConfig.apPassword.c_str());
    }

    bool startAPSTAMode()
    {
        auto &netConfig = configManager.getConfig().network;

        Serial.println("📡 Starting WiFi AP+STA mode");

        WiFi.mode(WIFI_AP_STA);

        // Запускаем AP
        if (!netConfig.apSsid.isEmpty())
        {
            WiFi.softAP(netConfig.apSsid.c_str(), netConfig.apPassword.c_str());
        }

        // Запускаем STA
        if (!netConfig.staSsid.isEmpty())
        {
            connectionInProgress = true;
            connectionStartTime = millis();
            WiFi.begin(netConfig.staSsid.c_str(), netConfig.staPassword.c_str());
        }

        return true;
    }
};