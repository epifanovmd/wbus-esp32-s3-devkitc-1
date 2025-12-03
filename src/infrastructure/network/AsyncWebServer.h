#pragma once
#include <Arduino.h>

#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "./domain/Events.h"
#include "../../application/HeaterController.h"
#include "../../application/ErrorsManager.h"
#include "../../application/DeviceInfoManager.h"
#include "../../application/SensorManager.h"

class AsyncApiServer
{
private:
    AsyncWebServer server;
    AsyncWebSocket ws;

    DeviceInfoManager &deviceInfoManager;
    SensorManager &sensorManager;
    ErrorsManager &errorsManager;
    HeaterController &heaterController;

    uint16_t port;
    bool enabled = false;

public:
    AsyncApiServer(DeviceInfoManager &deviceInfoMngr, SensorManager &sensorMngr,
                   ErrorsManager &errorsMngr, HeaterController &heaterCtrl,
                   uint16_t serverPort = 80) : server(serverPort),
                                               ws("/ws"),
                                               deviceInfoManager(deviceInfoMngr),
                                               sensorManager(sensorMngr),
                                               errorsManager(errorsMngr),
                                               heaterController(heaterCtrl),
                                               port(serverPort)
    {
        // Настройка WebSocket события
        ws.onEvent([this](AsyncWebSocket *server, AsyncWebSocketClient *client,
                          AwsEventType type, void *arg, uint8_t *data, size_t len)
                   { handleWebSocketEvent(server, client, type, arg, data, len); });
    }

    bool initialize()
    {
        if (!LittleFS.begin(true))
        {
            Serial.println("❌ LittleFS initialization failed");
            return false;
        }

        // Проверяем наличие index.html
        if (!LittleFS.exists("/index.html"))
        {
            Serial.println("⚠️  index.html not found");
        }

        // Настройка обработчиков событий EventBus
        setupEventHandlers();

        // Регистрация обработчиков WebSocket
        server.addHandler(&ws);

        // Настройка CORS для всех запросов
        DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");
        DefaultHeaders::Instance().addHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
        DefaultHeaders::Instance().addHeader("Access-Control-Allow-Headers", "Content-Type");

        // Настройка обработчиков маршрутов
        setupEndpoints();

        // Обработка статических файлов - УПРОЩЕННАЯ ВЕРСИЯ
        server.serveStatic("/", LittleFS, "/")
            .setTryGzipFirst(false)
            .setDefaultFile("index.html")
            .setCacheControl("max-age=3600"); // Кэш на 1 час

        // Специальный обработчик для корня
        // server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request)
        //           { handleRoot(request); });

        // Обработчик для не найденных маршрутов
        server.onNotFound([this](AsyncWebServerRequest *request)
                          { handleNotFound(request); });

        // Запуск сервера
        server.begin();
        enabled = true;

        Serial.println("✅ Async WebServer started on port " + String(port));
        Serial.println("✅ WebSocket available at ws://" + WiFi.softAPIP().toString() + "/ws");
        printAvailableEndpoints();

        return true;
    }

    void process()
    {
        ws.cleanupClients();
    }

    void broadcastJson(EventType eventType,
                       const String &json)
    {
        if (!enabled || ws.count() == 0)
            return;

        // Получаем строковое представление типа события из EventBus
        EventBus &eventBus = EventBus::getInstance();
        String eventTypeStr = eventBus.getEventTypeString(eventType);

        // Формируем JSON сообщение
        String message = "{";
        message += "\"type\":\"" + eventTypeStr + "\",";
        message += "\"data\":" + json;
        message += "}";

        // Отправляем всем подключенным клиентам
        ws.textAll(message);
    }

    bool isWebSocketConnected()
    {
        return ws.count() > 0;
    }

private:
    void setupEndpoints()
    {
        // =========================================================================
        // УПРАВЛЕНИЕ ПОДКЛЮЧЕНИЕМ
        // =========================================================================
        server.on("/api/connect", HTTP_POST, [this](AsyncWebServerRequest *request)
                  { handleConnect(request); });
        server.on("/api/disconnect", HTTP_POST, [this](AsyncWebServerRequest *request)
                  { handleDisconnect(request); });
        server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request)
                  { handleGetStatus(request); });

        // =========================================================================
        // ОСНОВНЫЕ КОМАНДЫ УПРАВЛЕНИЯ
        // =========================================================================
        server.on("/api/start/parking", HTTP_POST, [this](AsyncWebServerRequest *request)
                  { handleStartParking(request); });
        server.on("/api/start/ventilation", HTTP_POST, [this](AsyncWebServerRequest *request)
                  { handleStartVentilation(request); });
        server.on("/api/start/supplemental", HTTP_POST, [this](AsyncWebServerRequest *request)
                  { handleStartSupplemental(request); });
        server.on("/api/start/boost", HTTP_POST, [this](AsyncWebServerRequest *request)
                  { handleStartBoost(request); });
        server.on("/api/control/circulation-pump", HTTP_POST, [this](AsyncWebServerRequest *request)
                  { handleControlCirculationPump(request); });
        server.on("/api/shutdown", HTTP_POST, [this](AsyncWebServerRequest *request)
                  { handleShutdown(request); });

        // =========================================================================
        // ТЕСТИРОВАНИЕ КОМПОНЕНТОВ
        // =========================================================================

        server.addHandler(new AsyncCallbackJsonWebHandler("/api/test/combustion-fan",
                                                          [this](AsyncWebServerRequest *request, JsonVariant &json)
                                                          {
                                                              handleTestCombustionFan(request, json);
                                                          }));

        server.addHandler(new AsyncCallbackJsonWebHandler("/api/test/fuel-pump",
                                                          [this](AsyncWebServerRequest *request, JsonVariant &json)
                                                          {
                                                              handleTestFuelPump(request, json);
                                                          }));

        server.addHandler(new AsyncCallbackJsonWebHandler("/api/test/glow-plug",
                                                          [this](AsyncWebServerRequest *request, JsonVariant &json)
                                                          {
                                                              handleTestGlowPlug(request, json);
                                                          }));

        server.addHandler(new AsyncCallbackJsonWebHandler("/api/test/circulation-pump",
                                                          [this](AsyncWebServerRequest *request, JsonVariant &json)
                                                          {
                                                              handleTestCirculationPump(request, json);
                                                          }));

        server.addHandler(new AsyncCallbackJsonWebHandler("/api/test/vehicle-fan",
                                                          [this](AsyncWebServerRequest *request, JsonVariant &json)
                                                          {
                                                              handleTestVehicleFan(request, json);
                                                          }));

        server.addHandler(new AsyncCallbackJsonWebHandler("/api/test/solenoid",
                                                          [this](AsyncWebServerRequest *request, JsonVariant &json)
                                                          {
                                                              handleTestSolenoid(request, json);
                                                          }));

        server.addHandler(new AsyncCallbackJsonWebHandler("/api/test/fuel-preheating",
                                                          [this](AsyncWebServerRequest *request, JsonVariant &json)
                                                          {
                                                              // Обработка запроса
                                                              handleTestFuelPreheating(request, json);
                                                          }));

        // =========================================================================
        // ЧТЕНИЕ ДАННЫХ
        // =========================================================================
        server.on("/api/device/info", HTTP_GET, [this](AsyncWebServerRequest *request)
                  { handleGetDeviceInfo(request); });
        server.on("/api/sensors/data", HTTP_GET, [this](AsyncWebServerRequest *request)
                  { handleGetSensorsData(request); });
        server.on("/api/errors", HTTP_GET, [this](AsyncWebServerRequest *request)
                  { handleGetErrors(request); });
        server.on("/api/errors/clear", HTTP_POST, [this](AsyncWebServerRequest *request)
                  { handleClearErrors(request); });

        // =========================================================================
        // ДОПОЛНИТЕЛЬНЫЕ МАРШРУТЫ
        // =========================================================================
        server.on("/api/system/info", HTTP_GET, [this](AsyncWebServerRequest *request)
                  { handleSystemInfo(request); });
        server.on("/api/system/restart", HTTP_POST, [this](AsyncWebServerRequest *request)
                  { handleSystemRestart(request); });
    }

    void setupEventHandlers()
    {
        EventBus &eventBus = EventBus::getInstance();

        eventBus.subscribe(EventType::COMMAND_NAK_RESPONSE,
                           [this](const Event &event)
                           {
                               const auto &nakEvent = static_cast<
                                   const TypedEvent<NakResponseEvent> &>(event);
                               broadcastJson(EventType::COMMAND_NAK_RESPONSE, nakEvent.data.toJson());
                           });

        // =========================================================================
        // СОБЫТИЯ СОСТОЯНИЯ ПОДКЛЮЧЕНИЯ И СИСТЕМЫ
        // =========================================================================

        eventBus.subscribe(EventType::HEATER_STATE_CHANGED,
                           [this](const Event &event)
                           {
                               const auto &stateEvent = static_cast<
                                   const TypedEvent<HeaterStateChangedEvent> &>(event);
                               broadcastJson(EventType::HEATER_STATE_CHANGED, stateEvent.data.toJson());
                           });

        eventBus.subscribe(EventType::CONNECTION_STATE_CHANGED,
                           [this](const Event &event)
                           {
                               const auto &connectionEvent = static_cast<
                                   const TypedEvent<ConnectionStateChangedEvent> &>(event);
                               broadcastJson(EventType::CONNECTION_STATE_CHANGED, connectionEvent.data.toJson());
                           });

        eventBus.subscribe(EventType::KEEP_ALLIVE_SENT,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::KEEP_ALLIVE_SENT, "{}");
                           });

        // =========================================================================
        // СОБЫТИЯ КОМАНД И ОТВЕТОВ
        // =========================================================================

        eventBus.subscribe(EventType::COMMAND_SENT_TIMEOUT,
                           [this](const Event &event)
                           {
                               const auto &timeoutEvent = static_cast<
                                   const TypedEvent<ConnectionTimeoutEvent> &>(event);
                               broadcastJson(EventType::COMMAND_SENT_TIMEOUT, timeoutEvent.data.toJson());
                           });

        eventBus.subscribe(EventType::COMMAND_SENT_ERRROR,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::COMMAND_SENT_ERRROR, "\"" + event.source + "\"");
                           });

        // =========================================================================
        // СОБЫТИЯ ПЕРЕХВАТА K-LINE ПАКЕТОВ
        // =========================================================================

        eventBus.subscribe(EventType::TX_RECEIVED,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::TX_RECEIVED, "\"" + event.source + "\"");
                           });

        eventBus.subscribe(EventType::RX_RECEIVED,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::RX_RECEIVED, "\"" + event.source + "\"");
                           });

        eventBus.subscribe(EventType::COMMAND_RECEIVED,
                           [this](const Event &event)
                           {
                               const auto &cmdEvent = static_cast<const TypedEvent<CommandReceivedEvent> &>(event);

                               broadcastJson(EventType::COMMAND_RECEIVED, cmdEvent.data.toJson());
                           });

        // =========================================================================
        // СОБЫТИЯ ИНФОРМАЦИИ ОБ УСТРОЙСТВЕ
        // =========================================================================

        eventBus.subscribe(EventType::WBUS_VERSION,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::WBUS_VERSION, "\"" + String(event.source) + "\"");
                           });

        eventBus.subscribe(EventType::DEVICE_NAME,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::DEVICE_NAME, "\"" + String(event.source) + "\"");
                           });

        eventBus.subscribe(EventType::WBUS_CODE,
                           [this](const Event &event)
                           {
                               const auto &codeEvent = static_cast<
                                   const TypedEvent<DecodedWBusCode> &>(event);
                               broadcastJson(EventType::WBUS_CODE, codeEvent.data.toJson());
                           });

        eventBus.subscribe(EventType::DEVICE_ID,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::DEVICE_ID, "\"" + String(event.source) + "\"");
                           });

        eventBus.subscribe(EventType::CONTRALLER_MANUFACTURE_DATE,
                           [this](const Event &event)
                           {
                               const auto &dateEvent = static_cast<
                                   const TypedEvent<DecodedManufactureDate> &>(event);
                               broadcastJson(EventType::CONTRALLER_MANUFACTURE_DATE, dateEvent.data.toJson());
                           });

        eventBus.subscribe(EventType::HEATER_MANUFACTURE_DATE,
                           [this](const Event &event)
                           {
                               const auto &dateEvent = static_cast<
                                   const TypedEvent<DecodedManufactureDate> &>(event);
                               broadcastJson(EventType::HEATER_MANUFACTURE_DATE, dateEvent.data.toJson());
                           });

        eventBus.subscribe(EventType::CUSTOMER_ID,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::CUSTOMER_ID, "\"" + String(event.source) + "\"");
                           });

        eventBus.subscribe(EventType::SERIAL_NUMBER,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::SERIAL_NUMBER, "\"" + String(event.source) + "\"");
                           });

        // =========================================================================
        // СОБЫТИЯ ОШИБОК WEBASTO
        // =========================================================================

        eventBus.subscribe(EventType::WBUS_ERRORS,
                           [this](const Event &event)
                           {
                               const auto &errorsEvent = static_cast<
                                   const TypedEvent<ErrorCollection> &>(event);
                               broadcastJson(EventType::WBUS_ERRORS, errorsEvent.data.toJson());
                           });

        // =========================================================================
        // СОБЫТИЯ ДАТЧИКОВ
        // =========================================================================

        eventBus.subscribe(EventType::SENSOR_OPERATIONAL_INFO,
                           [this](const Event &event)
                           {
                               const auto &sensorEvent = static_cast<
                                   const TypedEvent<OperationalMeasurements> &>(event);
                               broadcastJson(EventType::SENSOR_OPERATIONAL_INFO, sensorEvent.data.toJson());
                           });

        eventBus.subscribe(EventType::SENSOR_ON_OFF_FLAGS,
                           [this](const Event &event)
                           {
                               const auto &flagsEvent = static_cast<
                                   const TypedEvent<OnOffFlags> &>(event);
                               broadcastJson(EventType::SENSOR_ON_OFF_FLAGS, flagsEvent.data.toJson());
                           });

        eventBus.subscribe(EventType::SENSOR_STATUS_FLAGS,
                           [this](const Event &event)
                           {
                               const auto &statusEvent = static_cast<
                                   const TypedEvent<StatusFlags> &>(event);
                               broadcastJson(EventType::SENSOR_STATUS_FLAGS, statusEvent.data.toJson());
                           });

        eventBus.subscribe(EventType::SENSOR_OPERATING_STATE,
                           [this](const Event &event)
                           {
                               const auto &stateEvent = static_cast<
                                   const TypedEvent<OperatingState> &>(event);
                               broadcastJson(EventType::SENSOR_OPERATING_STATE, stateEvent.data.toJson());
                           });

        eventBus.subscribe(EventType::SENSOR_SUBSYSTEM_STATE,
                           [this](const Event &event)
                           {
                               const auto &subsystemEvent = static_cast<
                                   const TypedEvent<SubsystemsStatus> &>(event);
                               broadcastJson(EventType::SENSOR_SUBSYSTEM_STATE, subsystemEvent.data.toJson());
                           });

        eventBus.subscribe(EventType::FUEL_SETTINGS,
                           [this](const Event &event)
                           {
                               const auto &fuelEvent = static_cast<
                                   const TypedEvent<FuelSettings> &>(event);
                               broadcastJson(EventType::FUEL_SETTINGS, fuelEvent.data.toJson());
                           });

        eventBus.subscribe(EventType::SENSOR_OPERATING_TIMES,
                           [this](const Event &event)
                           {
                               const auto &operatingTimes = static_cast<
                                   const TypedEvent<OperatingTimes> &>(event);
                               broadcastJson(EventType::SENSOR_OPERATING_TIMES, operatingTimes.data.toJson());
                           });

        eventBus.subscribe(EventType::FUEL_PREWARMING,
                           [this](const Event &event)
                           {
                               const auto &fuelPrewarming = static_cast<
                                   const TypedEvent<FuelPrewarming> &>(event);
                               broadcastJson(EventType::FUEL_PREWARMING, fuelPrewarming.data.toJson());
                           });

        eventBus.subscribe(EventType::BURNING_DURATION_STATS,
                           [this](const Event &event)
                           {
                               const auto &burningDurationEvent = static_cast<
                                   const TypedEvent<BurningDuration> &>(event);
                               broadcastJson(EventType::BURNING_DURATION_STATS, burningDurationEvent.data.toJson());
                           });

        eventBus.subscribe(EventType::START_COUNTERS,
                           [this](const Event &event)
                           {
                               const auto &startCounters = static_cast<
                                   const TypedEvent<StartCounters> &>(event);
                               broadcastJson(EventType::START_COUNTERS, startCounters.data.toJson());
                           });

        // =========================================================================
        // СОБЫТИЯ ТЕСТИРОВАНИЯ КОМПОНЕНТОВ
        // =========================================================================

        eventBus.subscribe(EventType::TEST_COMBUSTION_FAN_STARTED,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::TEST_COMBUSTION_FAN_STARTED, "{\"component\":\"combustionFan\",\"status\":\"started\"}");
                           });

        eventBus.subscribe(EventType::TEST_COMBUSTION_FAN_FAILED,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::TEST_COMBUSTION_FAN_FAILED, "{\"component\":\"combustionFan\",\"status\":\"failed\"}");
                           });

        eventBus.subscribe(EventType::TEST_FUEL_PUMP_STARTED,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::TEST_FUEL_PUMP_STARTED, "{\"component\":\"fuelPump\",\"status\":\"started\"}");
                           });

        eventBus.subscribe(EventType::TEST_FUEL_PUMP_FAILED,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::TEST_FUEL_PUMP_FAILED, "{\"component\":\"fuelPump\",\"status\":\"failed\"}");
                           });

        eventBus.subscribe(EventType::TEST_GLOW_PLUG_STARTED,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::TEST_GLOW_PLUG_STARTED, "{\"component\":\"glowPlug\",\"status\":\"started\"}");
                           });

        eventBus.subscribe(EventType::TEST_GLOW_PLUG_FAILED,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::TEST_GLOW_PLUG_FAILED, "{\"component\":\"glowPlug\",\"status\":\"failed\"}");
                           });

        eventBus.subscribe(EventType::TEST_CIRCULATION_PUMP_STARTED,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::TEST_CIRCULATION_PUMP_STARTED, "{\"component\":\"circulationPump\",\"status\":\"started\"}");
                           });

        eventBus.subscribe(EventType::TEST_CIRCULATION_PUMP_FAILED,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::TEST_CIRCULATION_PUMP_FAILED, "{\"component\":\"circulationPump\",\"status\":\"failed\"}");
                           });

        eventBus.subscribe(EventType::TEST_VEHICLE_FAN_STARTED,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::TEST_VEHICLE_FAN_STARTED, "{\"component\":\"vehicleFan\",\"status\":\"started\"}");
                           });

        eventBus.subscribe(EventType::TEST_VEHICLE_FAN_FAILED,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::TEST_VEHICLE_FAN_FAILED, "{\"component\":\"vehicleFan\",\"status\":\"failed\"}");
                           });

        eventBus.subscribe(EventType::TEST_SOLENOID_STARTED,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::TEST_SOLENOID_STARTED, "{\"component\":\"solenoid\",\"status\":\"started\"}");
                           });

        eventBus.subscribe(EventType::TEST_SOLENOID_FAILED,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::TEST_SOLENOID_FAILED, "{\"component\":\"solenoid\",\"status\":\"failed\"}");
                           });

        eventBus.subscribe(EventType::TEST_FUEL_PREHEATING_STARTED,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::TEST_FUEL_PREHEATING_STARTED, "{\"component\":\"fuelPreheating\",\"status\":\"started\"}");
                           });

        eventBus.subscribe(EventType::TEST_FUEL_PREHEATING_FAILED,
                           [this](const Event &event)
                           {
                               broadcastJson(EventType::TEST_FUEL_PREHEATING_FAILED, "{\"component\":\"fuelPreheating\",\"status\":\"failed\"}");
                           });
    }

    // =========================================================================
    // ОБРАБОТЧИКИ HTTP ЗАПРОСОВ
    // =========================================================================

    void handleRoot(AsyncWebServerRequest *request)
    {
        // Прямая отдача index.html без поиска gzip версий
        if (LittleFS.exists("/index.html"))
        {
            request->send(LittleFS, "/index.html", "text/html");
        }
        else
        {
            sendJsonResponse(request, "{\"error\":\"index.html not found\"}", 404);
        }
    }

    void handleConnect(AsyncWebServerRequest *request)
    {
        heaterController.connect();
        sendJsonResponse(request, "{\"status\":\"connecting\"}");
    }

    void handleDisconnect(AsyncWebServerRequest *request)
    {
        heaterController.disconnect();
        sendJsonResponse(request, "{\"status\":\"disconnected\"}");
    }

    void handleGetStatus(AsyncWebServerRequest *request)
    {
        HeaterStatus status = heaterController.getStatus();
        DynamicJsonDocument doc(512);
        doc["heaterState"] = status.getStateName();
        doc["connectionState"] = status.getConnectionName();
        doc["isConnected"] = status.isConnected();

        String json;
        serializeJson(doc, json);
        sendJsonResponse(request, json);
    }

    void handleStartParking(AsyncWebServerRequest *request)
    {
        int minutes = getIntParam(request, "minutes", 60);
        heaterController.startParkingHeat(minutes);
        sendJsonResponse(request, "{\"status\":\"started\",\"mode\":\"parking\",\"minutes\":" + String(minutes) + "}");
    }

    void handleStartVentilation(AsyncWebServerRequest *request)
    {
        int minutes = getIntParam(request, "minutes", 60);
        heaterController.startVentilation(minutes);
        sendJsonResponse(request, "{\"status\":\"started\",\"mode\":\"ventilation\",\"minutes\":" + String(minutes) + "}");
    }

    void handleStartSupplemental(AsyncWebServerRequest *request)
    {
        int minutes = getIntParam(request, "minutes", 60);
        heaterController.startSupplementalHeat(minutes);
        sendJsonResponse(request, "{\"status\":\"started\",\"mode\":\"supplemental\",\"minutes\":" + String(minutes) + "}");
    }

    void handleStartBoost(AsyncWebServerRequest *request)
    {
        int minutes = getIntParam(request, "minutes", 60);
        heaterController.startBoostMode(minutes);
        sendJsonResponse(request, "{\"status\":\"started\",\"mode\":\"boost\",\"minutes\":" + String(minutes) + "}");
    }

    void handleControlCirculationPump(AsyncWebServerRequest *request)
    {
        bool enable = getBoolParam(request, "enable", false);
        heaterController.controlCirculationPump(enable);
        sendJsonResponse(request, "{\"status\":\"updated\",\"pumpEnabled\":" + String(enable ? "true" : "false") + "}");
    }

    void handleShutdown(AsyncWebServerRequest *request)
    {
        heaterController.shutdown();
        sendJsonResponse(request, "{\"status\":\"shutdown\"}");
    }

    void handleTestCombustionFan(AsyncWebServerRequest *request, JsonVariant &json)
    {
        if (!json.containsKey("seconds") || !json.containsKey("power"))
        {
            request->send(400, "application/json", "{\"error\":\"Missing required fields\",\"required\":[\"seconds\",\"power\"]}");
            return;
        }

        int seconds = json["seconds"] | 10;
        int power = json["power"] | 50;

        seconds = constrain(seconds, 1, 30);
        power = constrain(power, 1, 100);
        heaterController.testCombustionFan(seconds, power);
        sendJsonResponse(request, "{\"status\":\"testing\",\"component\":\"combustionFan\",\"seconds\":" + String(seconds) + ",\"power\":" + String(power) + "}");
    }

    void handleTestFuelPump(AsyncWebServerRequest *request, JsonVariant &json)
    {
        if (!json.containsKey("seconds") || !json.containsKey("frequency"))
        {
            request->send(400, "application/json", "{\"error\":\"Missing required fields\",\"required\":[\"seconds\",\"frequency\"]}");
            return;
        }

        int seconds = json["seconds"] | 10;
        int frequency = json["frequency"] | 50;

        seconds = constrain(seconds, 1, 30);
        frequency = constrain(frequency, 1, 100);
        heaterController.testFuelPump(seconds, frequency);
        sendJsonResponse(request, "{\"status\":\"testing\",\"component\":\"fuelPump\",\"seconds\":" + String(seconds) + ",\"frequency\":" + String(frequency) + "}");
    }

    void handleTestGlowPlug(AsyncWebServerRequest *request, JsonVariant &json)
    {
        if (!json.containsKey("seconds") || !json.containsKey("power"))
        {
            request->send(400, "application/json", "{\"error\":\"Missing required fields\",\"required\":[\"seconds\",\"power\"]}");
            return;
        }

        int seconds = json["seconds"] | 10;
        int power = json["power"] | 50;

        seconds = constrain(seconds, 1, 30);
        power = constrain(power, 1, 100);
        heaterController.testGlowPlug(seconds, power);
        sendJsonResponse(request, "{\"status\":\"testing\",\"component\":\"glowGlug\",\"seconds\":" + String(seconds) + ",\"power\":" + String(power) + "}");
    }

    void handleTestCirculationPump(AsyncWebServerRequest *request, JsonVariant &json)
    {
        if (!json.containsKey("seconds") || !json.containsKey("power"))
        {
            request->send(400, "application/json", "{\"error\":\"Missing required fields\",\"required\":[\"seconds\",\"power\"]}");
            return;
        }

        int seconds = json["seconds"] | 10;
        int power = json["power"] | 50;

        seconds = constrain(seconds, 1, 30);
        power = constrain(power, 1, 100);
        heaterController.testCirculationPump(seconds, power);
        sendJsonResponse(request, "{\"status\":\"testing\",\"component\":\"circulationPump\",\"seconds\":" + String(seconds) + ",\"power\":" + String(power) + "}");
    }

    void handleTestVehicleFan(AsyncWebServerRequest *request, JsonVariant &json)
    {
        if (!json.containsKey("seconds"))
        {
            request->send(400, "application/json", "{\"error\":\"Missing required fields\",\"required\":[\"seconds\"]}");
            return;
        }

        int seconds = json["seconds"] | 10;
        seconds = constrain(seconds, 1, 30);
        heaterController.testVehicleFan(seconds);
        sendJsonResponse(request, "{\"status\":\"testing\",\"component\":\"vehicleFan\",\"seconds\":" + String(seconds) + "}");
    }

    void handleTestSolenoid(AsyncWebServerRequest *request, JsonVariant &json)
    {
        if (!json.containsKey("seconds"))
        {
            request->send(400, "application/json", "{\"error\":\"Missing required fields\",\"required\":[\"seconds\"]}");
            return;
        }

        int seconds = json["seconds"] | 10;

        seconds = constrain(seconds, 1, 30);
        heaterController.testSolenoidValve(seconds);
        sendJsonResponse(request, "{\"status\":\"testing\",\"component\":\"solenoid\",\"seconds\":" + String(seconds) + "}");
    }

    void handleTestFuelPreheating(AsyncWebServerRequest *request, JsonVariant &json)
    {
        if (!json.containsKey("seconds") || !json.containsKey("power"))
        {
            request->send(400, "application/json", "{\"error\":\"Missing required fields\",\"required\":[\"seconds\",\"power\"]}");
            return;
        }

        int seconds = json["seconds"] | 10;
        int power = json["power"] | 50;

        seconds = constrain(seconds, 1, 30);
        power = constrain(power, 1, 100);
        heaterController.testFuelPreheating(seconds, power);
        sendJsonResponse(request, "{\"status\":\"testing\",\"component\":\"fuelPreheating\",\"seconds\":" + String(seconds) + ",\"power\":" + String(power) + "}");
    }

    void handleGetDeviceInfo(AsyncWebServerRequest *request)
    {
        String json = deviceInfoManager.getDeviceInfoJson();
        sendJsonResponse(request, json);
    }

    void handleGetSensorsData(AsyncWebServerRequest *request)
    {
        String json = sensorManager.getAllSensorsJson();
        sendJsonResponse(request, json);
    }

    void handleGetErrors(AsyncWebServerRequest *request)
    {
        String json = errorsManager.getErrorsJson();
        sendJsonResponse(request, json);
    }

    void handleClearErrors(AsyncWebServerRequest *request)
    {
        heaterController.breakIfNeeded();
        errorsManager.resetErrors();
        sendJsonResponse(request, "{\"status\":\"cleared\"}");
    }

    void handleSystemInfo(AsyncWebServerRequest *request)
    {
        DynamicJsonDocument doc(512);
        doc["chipModel"] = ESP.getChipModel();
        doc["chipRevision"] = ESP.getChipRevision();
        doc["cpuFreqMhz"] = ESP.getCpuFreqMHz();
        doc["flashSize"] = ESP.getFlashChipSize();
        doc["heapFree"] = ESP.getFreeHeap();
        doc["heapTotal"] = ESP.getHeapSize();
        doc["psramFree"] = ESP.getFreePsram();
        doc["psramTotal"] = ESP.getPsramSize();
        doc["sketchSize"] = ESP.getSketchSize();
        doc["freeSketchSpace"] = ESP.getFreeSketchSpace();

        String json;
        serializeJson(doc, json);
        sendJsonResponse(request, json);
    }

    void handleSystemRestart(AsyncWebServerRequest *request)
    {
        sendJsonResponse(request, "{\"status\":\"restarting\"}");
        delay(100);
        ESP.restart();
    }

    void handleNotFound(AsyncWebServerRequest *request)
    {
        String path = request->url();
        if (path.startsWith("/api/"))
        {
            sendJsonResponse(request, "{\"error\":\"not_found\",\"endpoint\":\"" + path + "\"}", 404);
        }
        else
        {
            // Для не-API запросов возвращаем 404
            sendJsonResponse(request, "{\"error\":\"not_found\",\"path\":\"" + path + "\"}", 404);
        }
    }

    // =========================================================================
    // WEB SOCKET ОБРАБОТЧИКИ
    // =========================================================================

    void handleWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client,
                              AwsEventType type, void *arg, uint8_t *data, size_t len)
    {
        switch (type)
        {
        case WS_EVT_CONNECT:
        {
            IPAddress ip = client->remoteIP();
            Serial.printf("[WebSocket] Client #%u connected from %d.%d.%d.%d\n", client->id(), ip[0], ip[1], ip[2], ip[3]);

            // Отправляем начальное состояние при подключении
            sendInitialState(client);
            break;
        }
        case WS_EVT_DISCONNECT:
            Serial.printf("[WebSocket] Client #%u disconnected\n", client->id());
            break;
        case WS_EVT_DATA:
            handleWebSocketMessage(arg, data, len);
            break;
        case WS_EVT_PONG:
        case WS_EVT_ERROR:
            break;
        }
    }

    void handleWebSocketMessage(void *arg, uint8_t *data, size_t len)
    {
        AwsFrameInfo *info = (AwsFrameInfo *)arg;
        if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
        {
            data[len] = 0;
            String message = String((char *)data);

            // Здесь можно добавить обработку входящих сообщений от клиента
            // Например, команды управления нагревателем

            Serial.printf("[WebSocket] Received: %s\n", message.c_str());

            // Эхо-ответ для тестирования
            DynamicJsonDocument doc(256);
            doc["type"] = "echo";
            doc["message"] = "received: " + message;

            String json;
            serializeJson(doc, json);
            ws.textAll(json);
        }
    }

    void sendInitialState(AsyncWebSocketClient *client)
    {
        DynamicJsonDocument doc(512);
        doc["type"] = "welcome";
        doc["message"] = "Connected to Webasto Controller";
        doc["server"] = "ESP32 AsyncWebServer";
        doc["version"] = "1.0.0";

        HeaterStatus status = heaterController.getStatus();
        doc["heaterState"] = status.getStateName();
        doc["connectionState"] = status.getConnectionName();

        String json;
        serializeJson(doc, json);
        client->text(json);
    }

    // =========================================================================
    // ВСПОМОГАТЕЛЬНЫЕ МЕТОДЫ
    // =========================================================================

    void sendJsonResponse(AsyncWebServerRequest *request,
                          const String &json, int statusCode = 200)
    {
        AsyncWebServerResponse *resp = request->beginResponse(statusCode, "application/json", json);
        resp->addHeader("Access-Control-Allow-Origin", "*");
        request->send(resp);
    }

    int getIntParam(AsyncWebServerRequest *request,
                    const String &param, int defaultValue)
    {
        if (request->hasParam(param))
        {
            String value = request->getParam(param)->value();
            return value.toInt();
        }

        if (request->hasParam(param, true))
        {
            String value = request->getParam(param, true)->value();
            return value.toInt();
        }

        return defaultValue;
    }

    bool getBoolParam(AsyncWebServerRequest *request,
                      const String &param, bool defaultValue)
    {
        if (request->hasParam(param))
        {
            String value = request->getParam(param)->value();
            return value == "true" || value == "1";
        }

        if (request->hasParam(param, true))
        {
            String value = request->getParam(param, true)->value();
            return value == "true" || value == "1";
        }
        return defaultValue;
    }

    String getStringParam(AsyncWebServerRequest *request,
                          const String &param,
                          const String &defaultValue)
    {
        if (request->hasParam(param))
        {
            return request->getParam(param)->value();
        }

        if (request->hasParam(param, true))
        {
            return request->getParam(param, true)->value();
        }
        return defaultValue;
    }

    void printAvailableEndpoints()
    {
        Serial.println();
        Serial.println("🌐 Available API Endpoints:");
        Serial.println("  GET  /                    - Web interface");
        Serial.println("  WS   /ws                  - WebSocket connection");
        Serial.println();
        Serial.println("  POST /api/connect         - Connect to Webasto");
        Serial.println("  POST /api/disconnect      - Disconnect from Webasto");
        Serial.println("  GET  /api/status          - Get system status");
        Serial.println();
        Serial.println("  POST /api/start/parking   - Start parking heat");
        Serial.println("  POST /api/start/ventilation - Start ventilation");
        Serial.println("  POST /api/start/supplemental - Start supplemental heat");
        Serial.println("  POST /api/start/boost     - Start boost mode");
        Serial.println("  POST /api/shutdown        - Shutdown heater");
        Serial.println();
        Serial.println("  POST /api/test/combustion-fan - Test combustion fan");
        Serial.println("  POST /api/test/fuel-pump  - Test fuel pump");
        Serial.println("  POST /api/test/glow-plug  - Test glow plug");
        Serial.println("  POST /api/test/circulation-pump - Test circulation pump");
        Serial.println("  POST /api/test/vehicle-fan - Test vehicle fan");
        Serial.println("  POST /api/test/solenoid   - Test solenoid valve");
        Serial.println("  POST /api/test/fuel-preheating - Test fuel preheating");
        Serial.println();
        Serial.println("  GET  /api/device/info     - Get device information");
        Serial.println("  GET  /api/sensors/data    - Get sensors data");
        Serial.println("  GET  /api/errors          - Get errors list");
        Serial.println("  POST /api/errors/clear    - Clear errors");
        Serial.println();
        Serial.println("  GET  /api/system/info     - Get system information");
        Serial.println("  POST /api/system/restart  - Restart system");
        Serial.println();
    }
};