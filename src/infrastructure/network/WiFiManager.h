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

    // Флаги для обработки событий
    bool connectionInProgress = false;

public:
    WiFiManager(ConfigManager &config, EventBus &bus)
        : configManager(config), eventBus(bus) 
    {
        // Настраиваем обработчики событий WiFi
        setupWiFiEventHandlers();
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
        // Проверка таймаута подключения
        if (connectionInProgress && 
            currentState == WiFiState::CONNECTING &&
            millis() - connectionStartTime > CONNECTION_TIMEOUT)
        {
            Serial.println("⏰ Connection timeout");
            onConnectionTimeout();
        }

        // Автоматическое переподключение
        auto &netConfig = configManager.getConfig().network;
        if (currentState == WiFiState::DISCONNECTED && 
            millis() - lastConnectionAttempt > netConfig.reconnectInterval &&
            !netConfig.staSsid.isEmpty())
        {
            Serial.println("🔄 Attempting to reconnect...");
            lastConnectionAttempt = millis();
            
            // Сбрасываем флаг и начинаем заново
            connectionInProgress = true;
            connectionStartTime = millis();
            
            WiFi.reconnect();
            setState(WiFiState::CONNECTING, netConfig.staSsid, "", "Reconnecting...");
        }
    }

    // =========================================================================
    // ОБРАБОТЧИКИ СОБЫТИЙ WiFi
    // =========================================================================

private:
    void setupWiFiEventHandlers()
    {
        // События WiFi
        WiFi.onEvent([this](arduino_event_id_t event, arduino_event_info_t info) {
            this->handleWiFiEvent(event, info);
        });
    }

    void handleWiFiEvent(arduino_event_id_t event, arduino_event_info_t info)
    {
        switch (event)
        {
        // События подключения STA
        case ARDUINO_EVENT_WIFI_STA_START:
            Serial.println("📡 STA Started");
            connectionInProgress = true;
            connectionStartTime = millis();
            break;

        case ARDUINO_EVENT_WIFI_STA_CONNECTED:
            Serial.println("📡 STA Connected to SSID: " + String((char*)info.wifi_sta_connected.ssid));
            currentSsid = String((char*)info.wifi_sta_connected.ssid);
            break;

        case ARDUINO_EVENT_WIFI_STA_GOT_IP:
            onWiFiConnected(
                IPAddress(info.got_ip.ip_info.ip.addr).toString(),
                IPAddress(info.got_ip.ip_info.gw.addr).toString(),
                IPAddress(info.got_ip.ip_info.netmask.addr).toString()
            );
            break;

        case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
            onWiFiDisconnected(
                info.wifi_sta_disconnected.reason,
                String((char*)info.wifi_sta_disconnected.ssid)
            );
            break;

        // События точки доступа AP
        case ARDUINO_EVENT_WIFI_AP_START:
            Serial.println("📡 AP Started");
            onAPStarted();
            break;

        case ARDUINO_EVENT_WIFI_AP_STOP:
            Serial.println("📡 AP Stopped");
            break;

        // Общие события
        case ARDUINO_EVENT_WIFI_READY:
            Serial.println("📡 WiFi Ready");
            break;

        default:
            // Другие события не обрабатываем
            break;
        }
    }

    void onWiFiConnected(const String &ip, const String &gateway, const String &netmask)
    {
        connectionInProgress = false;
        currentIp = ip;

        // Запускаем mDNS
        startMDNS();

        setState(WiFiState::CONNECTED, currentSsid, ip,
                 "Successfully connected");

        Serial.println("✅ WiFi Connected!");
        Serial.println("  IP: " + ip);
        Serial.println("  Gateway: " + gateway);
        Serial.println("  Netmask: " + netmask);
        Serial.println("  RSSI: " + String(WiFi.RSSI()) + " dBm");
        Serial.println("  mDNS: " + getMDNSURL());
    }

    void onWiFiDisconnected(uint8_t reason, const String &ssid)
    {
        connectionInProgress = false;
        
        setState(WiFiState::DISCONNECTED, "", "", "Disconnected: " + getDisconnectReason(reason));
        
        Serial.println("🔌 WiFi Disconnected");
        Serial.println("  SSID: " + ssid);
        Serial.println("  Reason: " + getDisconnectReason(reason));

        // Запускаем mDNS только если остались в AP режиме
        if (WiFi.getMode() & WIFI_AP)
        {
            startMDNS();
        }
        else
        {
            stopMDNS();
        }
    }

    void onAPStarted()
    {
        currentIp = WiFi.softAPIP().toString();
        auto &netConfig = configManager.getConfig().network;
        
        startMDNS();
        setState(WiFiState::AP_MODE, netConfig.apSsid, currentIp, "Access Point started");
        
        Serial.println("📡 AP Mode Active");
        Serial.println("  IP: " + currentIp);
        Serial.println("  SSID: " + netConfig.apSsid);
    }

    void onConnectionTimeout()
    {
        connectionInProgress = false;
        setState(WiFiState::DISCONNECTED, "", "", "Connection timeout");
        
        Serial.println("⏰ Connection timeout exceeded");
        WiFi.disconnect(false);
    }

    String getDisconnectReason(uint8_t reason) const
    {
        switch (reason)
        {
        case WIFI_REASON_AUTH_EXPIRE:     return "Auth expired";
        case WIFI_REASON_AUTH_LEAVE:      return "Auth leave";
        case WIFI_REASON_ASSOC_EXPIRE:    return "Association expired";
        case WIFI_REASON_ASSOC_TOOMANY:   return "Too many associations";
        case WIFI_REASON_NOT_AUTHED:      return "Not authenticated";
        case WIFI_REASON_NOT_ASSOCED:     return "Not associated";
        case WIFI_REASON_ASSOC_LEAVE:     return "Association leave";
        case WIFI_REASON_ASSOC_NOT_AUTHED:return "Association not authenticated";
        case WIFI_REASON_DISASSOC_PWRCAP_BAD: return "Disassociate: power capability";
        case WIFI_REASON_DISASSOC_SUPCHAN_BAD:return "Disassociate: supported channels";
        case WIFI_REASON_IE_INVALID:      return "IE invalid";
        case WIFI_REASON_MIC_FAILURE:     return "MIC failure";
        case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT: return "4-way handshake timeout";
        case WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT: return "Group key update timeout";
        case WIFI_REASON_IE_IN_4WAY_DIFFERS: return "IE in 4-way differs";
        case WIFI_REASON_GROUP_CIPHER_INVALID: return "Group cipher invalid";
        case WIFI_REASON_PAIRWISE_CIPHER_INVALID: return "Pairwise cipher invalid";
        case WIFI_REASON_AKMP_INVALID:    return "AKMP invalid";
        case WIFI_REASON_UNSUPP_RSN_IE_VERSION: return "Unsupported RSN IE version";
        case WIFI_REASON_INVALID_RSN_IE_CAP: return "Invalid RSN IE capabilities";
        case WIFI_REASON_802_1X_AUTH_FAILED: return "802.1x authentication failed";
        case WIFI_REASON_CIPHER_SUITE_REJECTED: return "Cipher suite rejected";
        case WIFI_REASON_BEACON_TIMEOUT:  return "Beacon timeout";
        case WIFI_REASON_NO_AP_FOUND:     return "No AP found";
        case WIFI_REASON_AUTH_FAIL:       return "Authentication failed";
        case WIFI_REASON_ASSOC_FAIL:      return "Association failed";
        case WIFI_REASON_HANDSHAKE_TIMEOUT: return "Handshake timeout";
        case WIFI_REASON_CONNECTION_FAIL: return "Connection failed";
        default: return "Unknown reason: " + String(reason);
        }
    }

    // =========================================================================
    // ОСТАЛЬНЫЕ МЕТОДЫ (без изменений)
    // =========================================================================

public:
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
    // ПРИВАТНЫЕ МЕТОДЫ РЕЖИМОВ
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
        connectionInProgress = true;

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

        // AP события будут обработаны через WiFi.onEvent
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
            connectionInProgress = true;
            connectionStartTime = millis();
            
            WiFi.begin(netConfig.staSsid.c_str(), netConfig.staPassword.c_str());
            setState(WiFiState::CONNECTING, netConfig.staSsid, "", "Connecting...");
        }

        return true;
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