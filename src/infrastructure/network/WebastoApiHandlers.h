#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include "./domain/Events.h"
#include "./ApiHelpers.h"
#include "../../application/HeaterController.h"
#include "../../application/ErrorsManager.h"
#include "../../application/DeviceInfoManager.h"
#include "../../application/SensorManager.h"

class WebastoApiHandlers
{
private:
    AsyncWebServer &server;

    DeviceInfoManager &deviceInfoManager;
    SensorManager &sensorManager;
    ErrorsManager &errorsManager;
    HeaterController &heaterController;

public:
    WebastoApiHandlers(AsyncWebServer &serv,
                       DeviceInfoManager &deviceInfoMngr,
                       SensorManager &sensorMngr,
                       ErrorsManager &errorsMngr,
                       HeaterController &heaterCtrl)
        : server(serv),
          deviceInfoManager(deviceInfoMngr),
          sensorManager(sensorMngr),
          errorsManager(errorsMngr),
          heaterController(heaterCtrl) {}

    void setupEndpoints()
    {
        // =========================================================================
        // УПРАВЛЕНИЕ ПОДКЛЮЧЕНИЕМ
        // =========================================================================
        server.on("/api/connect", HTTP_POST,
                  [this](AsyncWebServerRequest *request)
                  { handleConnect(request); });

        server.on("/api/disconnect", HTTP_POST,
                  [this](AsyncWebServerRequest *request)
                  { handleDisconnect(request); });

        server.on("/api/status", HTTP_GET,
                  [this](AsyncWebServerRequest *request)
                  { handleGetStatus(request); });

        // =========================================================================
        // ОСНОВНЫЕ КОМАНДЫ УПРАВЛЕНИЯ
        // =========================================================================
        server.on("/api/start/parking", HTTP_POST,
                  [this](AsyncWebServerRequest *request)
                  { handleStartParking(request); });

        server.on("/api/start/ventilation", HTTP_POST,
                  [this](AsyncWebServerRequest *request)
                  { handleStartVentilation(request); });

        server.on("/api/start/supplemental", HTTP_POST,
                  [this](AsyncWebServerRequest *request)
                  { handleStartSupplemental(request); });

        server.on("/api/start/boost", HTTP_POST,
                  [this](AsyncWebServerRequest *request)
                  { handleStartBoost(request); });

        server.on("/api/control/circulation-pump", HTTP_POST,
                  [this](AsyncWebServerRequest *request)
                  { handleControlCirculationPump(request); });

        server.on("/api/shutdown", HTTP_POST,
                  [this](AsyncWebServerRequest *request)
                  { handleShutdown(request); });

        // =========================================================================
        // ТЕСТИРОВАНИЕ КОМПОНЕНТОВ
        // =========================================================================
        server.addHandler(new AsyncCallbackJsonWebHandler("/api/test/combustion-fan",
                                                          [this](AsyncWebServerRequest *request, JsonVariant &json)
                                                          { handleTestCombustionFan(request, json); }));

        server.addHandler(new AsyncCallbackJsonWebHandler("/api/test/fuel-pump",
                                                          [this](AsyncWebServerRequest *request, JsonVariant &json)
                                                          { handleTestFuelPump(request, json); }));

        server.addHandler(new AsyncCallbackJsonWebHandler("/api/test/glow-plug",
                                                          [this](AsyncWebServerRequest *request, JsonVariant &json)
                                                          { handleTestGlowPlug(request, json); }));

        server.addHandler(new AsyncCallbackJsonWebHandler("/api/test/circulation-pump",
                                                          [this](AsyncWebServerRequest *request, JsonVariant &json)
                                                          { handleTestCirculationPump(request, json); }));

        server.addHandler(new AsyncCallbackJsonWebHandler("/api/test/vehicle-fan",
                                                          [this](AsyncWebServerRequest *request, JsonVariant &json)
                                                          { handleTestVehicleFan(request, json); }));

        server.addHandler(new AsyncCallbackJsonWebHandler("/api/test/solenoid",
                                                          [this](AsyncWebServerRequest *request, JsonVariant &json)
                                                          { handleTestSolenoid(request, json); }));

        server.addHandler(new AsyncCallbackJsonWebHandler("/api/test/fuel-preheating",
                                                          [this](AsyncWebServerRequest *request, JsonVariant &json)
                                                          { handleTestFuelPreheating(request, json); }));

        // =========================================================================
        // ЧТЕНИЕ ДАННЫХ
        // =========================================================================
        server.on("/api/device/info", HTTP_GET,
                  [this](AsyncWebServerRequest *request)
                  { handleGetDeviceInfo(request); });

        server.on("/api/sensors/data", HTTP_GET,
                  [this](AsyncWebServerRequest *request)
                  { handleGetSensorsData(request); });

        server.on("/api/errors", HTTP_GET,
                  [this](AsyncWebServerRequest *request)
                  { handleGetErrors(request); });

        server.on("/api/errors/clear", HTTP_POST,
                  [this](AsyncWebServerRequest *request)
                  { handleClearErrors(request); });
    }

    // =========================================================================
    // ОБРАБОТЧИКИ HTTP ЗАПРОСОВ
    // =========================================================================

    void handleConnect(AsyncWebServerRequest *request)
    {
        heaterController.connect();
        ApiHelpers::sendJsonResponse(request, "{\"status\":\"connecting\"}");
    }

    void handleDisconnect(AsyncWebServerRequest *request)
    {
        heaterController.disconnect();
        ApiHelpers::sendJsonResponse(request, "{\"status\":\"disconnected\"}");
    }

    void handleGetStatus(AsyncWebServerRequest *request)
    {
        HeaterStatus status = heaterController.getStatus();
        DynamicJsonDocument doc(512);
        doc["heaterState"] = status.getStateName();
        doc["connectionState"] = status.getConnectionName();
        doc["isConnected"] = status.isConnected();

        ApiHelpers::sendJsonDocument(request, doc);
    }

    void handleStartParking(AsyncWebServerRequest *request)
    {
        int minutes = ApiHelpers::getIntParam(request, "minutes", 60);
        heaterController.startParkingHeat(minutes);

        DynamicJsonDocument doc(128);
        doc["status"] = "started";
        doc["mode"] = "parking";
        doc["minutes"] = minutes;

        ApiHelpers::sendJsonDocument(request, doc);
    }

    void handleStartVentilation(AsyncWebServerRequest *request)
    {
        int minutes = ApiHelpers::getIntParam(request, "minutes", 60);
        heaterController.startVentilation(minutes);

        DynamicJsonDocument doc(128);
        doc["status"] = "started";
        doc["mode"] = "ventilation";
        doc["minutes"] = minutes;

        ApiHelpers::sendJsonDocument(request, doc);
    }

    void handleStartSupplemental(AsyncWebServerRequest *request)
    {
        int minutes = ApiHelpers::getIntParam(request, "minutes", 60);
        heaterController.startSupplementalHeat(minutes);

        DynamicJsonDocument doc(128);
        doc["status"] = "started";
        doc["mode"] = "supplemental";
        doc["minutes"] = minutes;

        ApiHelpers::sendJsonDocument(request, doc);
    }

    void handleStartBoost(AsyncWebServerRequest *request)
    {
        int minutes = ApiHelpers::getIntParam(request, "minutes", 60);
        heaterController.startBoostMode(minutes);

        DynamicJsonDocument doc(128);
        doc["status"] = "started";
        doc["mode"] = "boost";
        doc["minutes"] = minutes;

        ApiHelpers::sendJsonDocument(request, doc);
    }

    void handleControlCirculationPump(AsyncWebServerRequest *request)
    {
        bool enable = ApiHelpers::getBoolParam(request, "enable", false);
        heaterController.controlCirculationPump(enable);

        DynamicJsonDocument doc(128);
        doc["status"] = "updated";
        doc["pumpEnabled"] = enable;

        ApiHelpers::sendJsonDocument(request, doc);
    }

    void handleShutdown(AsyncWebServerRequest *request)
    {
        heaterController.shutdown();
        ApiHelpers::sendJsonResponse(request, "{\"status\":\"shutdown\"}");
    }

    void handleTestCombustionFan(AsyncWebServerRequest *request, JsonVariant &json)
    {
        if (!json.containsKey("seconds") || !json.containsKey("power"))
        {
            ApiHelpers::sendJsonError(request, "Missing required fields: seconds, power", 400);
            return;
        }

        int seconds = json["seconds"] | 10;
        int power = json["power"] | 50;

        seconds = constrain(seconds, 1, 30);
        power = constrain(power, 1, 100);
        heaterController.testCombustionFan(seconds, power);

        DynamicJsonDocument doc(128);
        doc["status"] = "testing";
        doc["component"] = "combustionFan";
        doc["seconds"] = seconds;
        doc["power"] = power;

        ApiHelpers::sendJsonDocument(request, doc);
    }

    void handleTestFuelPump(AsyncWebServerRequest *request, JsonVariant &json)
    {
        if (!json.containsKey("seconds") || !json.containsKey("frequency"))
        {
            ApiHelpers::sendJsonError(request, "Missing required fields: seconds, frequency", 400);
            return;
        }

        int seconds = json["seconds"] | 10;
        int frequency = json["frequency"] | 50;

        seconds = constrain(seconds, 1, 30);
        frequency = constrain(frequency, 1, 100);
        heaterController.testFuelPump(seconds, frequency);

        DynamicJsonDocument doc(128);
        doc["status"] = "testing";
        doc["component"] = "fuelPump";
        doc["seconds"] = seconds;
        doc["frequency"] = frequency;

        ApiHelpers::sendJsonDocument(request, doc);
    }

    void handleTestGlowPlug(AsyncWebServerRequest *request, JsonVariant &json)
    {
        if (!json.containsKey("seconds") || !json.containsKey("power"))
        {
            ApiHelpers::sendJsonError(request, "Missing required fields: seconds, power", 400);
            return;
        }

        int seconds = json["seconds"] | 10;
        int power = json["power"] | 50;

        seconds = constrain(seconds, 1, 30);
        power = constrain(power, 1, 100);
        heaterController.testGlowPlug(seconds, power);

        DynamicJsonDocument doc(128);
        doc["status"] = "testing";
        doc["component"] = "glowPlug";
        doc["seconds"] = seconds;
        doc["power"] = power;

        ApiHelpers::sendJsonDocument(request, doc);
    }

    void handleTestCirculationPump(AsyncWebServerRequest *request, JsonVariant &json)
    {
        if (!json.containsKey("seconds") || !json.containsKey("power"))
        {
            ApiHelpers::sendJsonError(request, "Missing required fields: seconds, power", 400);
            return;
        }

        int seconds = json["seconds"] | 10;
        int power = json["power"] | 50;

        seconds = constrain(seconds, 1, 30);
        power = constrain(power, 1, 100);
        heaterController.testCirculationPump(seconds, power);

        DynamicJsonDocument doc(128);
        doc["status"] = "testing";
        doc["component"] = "circulationPump";
        doc["seconds"] = seconds;
        doc["power"] = power;

        ApiHelpers::sendJsonDocument(request, doc);
    }

    void handleTestVehicleFan(AsyncWebServerRequest *request, JsonVariant &json)
    {
        if (!json.containsKey("seconds"))
        {
            ApiHelpers::sendJsonError(request, "Missing required field: seconds", 400);
            return;
        }

        int seconds = json["seconds"] | 10;
        seconds = constrain(seconds, 1, 30);
        heaterController.testVehicleFan(seconds);

        DynamicJsonDocument doc(128);
        doc["status"] = "testing";
        doc["component"] = "vehicleFan";
        doc["seconds"] = seconds;

        ApiHelpers::sendJsonDocument(request, doc);
    }

    void handleTestSolenoid(AsyncWebServerRequest *request, JsonVariant &json)
    {
        if (!json.containsKey("seconds"))
        {
            ApiHelpers::sendJsonError(request, "Missing required field: seconds", 400);
            return;
        }

        int seconds = json["seconds"] | 10;
        seconds = constrain(seconds, 1, 30);
        heaterController.testSolenoidValve(seconds);

        DynamicJsonDocument doc(128);
        doc["status"] = "testing";
        doc["component"] = "solenoid";
        doc["seconds"] = seconds;

        ApiHelpers::sendJsonDocument(request, doc);
    }

    void handleTestFuelPreheating(AsyncWebServerRequest *request, JsonVariant &json)
    {
        if (!json.containsKey("seconds") || !json.containsKey("power"))
        {
            ApiHelpers::sendJsonError(request, "Missing required fields: seconds, power", 400);
            return;
        }

        int seconds = json["seconds"] | 10;
        int power = json["power"] | 50;

        seconds = constrain(seconds, 1, 30);
        power = constrain(power, 1, 100);
        heaterController.testFuelPreheating(seconds, power);

        DynamicJsonDocument doc(128);
        doc["status"] = "testing";
        doc["component"] = "fuelPreheating";
        doc["seconds"] = seconds;
        doc["power"] = power;

        ApiHelpers::sendJsonDocument(request, doc);
    }

    void handleGetDeviceInfo(AsyncWebServerRequest *request)
    {
        String json = deviceInfoManager.getDeviceInfoJson();
        ApiHelpers::sendJsonResponse(request, json);
    }

    void handleGetSensorsData(AsyncWebServerRequest *request)
    {
        String json = sensorManager.getAllSensorsJson();
        ApiHelpers::sendJsonResponse(request, json);
    }

    void handleGetErrors(AsyncWebServerRequest *request)
    {
        String json = errorsManager.getErrorsJson();
        ApiHelpers::sendJsonResponse(request, json);
    }

    void handleClearErrors(AsyncWebServerRequest *request)
    {
        heaterController.breakIfNeeded();
        errorsManager.resetErrors();
        ApiHelpers::sendJsonResponse(request, "{\"status\":\"cleared\"}");
    }
};