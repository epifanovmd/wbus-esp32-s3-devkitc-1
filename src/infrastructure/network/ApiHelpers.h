#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

class ApiHelpers
{
public:
    static void sendJsonResponse(AsyncWebServerRequest *request,
                                 const String &json,
                                 int statusCode = 200)
    {
        AsyncWebServerResponse *resp = request->beginResponse(statusCode,
                                                              "application/json",
                                                              json);
        resp->addHeader("Access-Control-Allow-Origin", "*");
        request->send(resp);
    }

    static void sendJsonDocument(AsyncWebServerRequest *request,
                                 DynamicJsonDocument &doc,
                                 int statusCode = 200)
    {
        String json;
        serializeJson(doc, json);
        sendJsonResponse(request, json, statusCode);
    }

    static void sendJsonError(AsyncWebServerRequest *request,
                              const String &message,
                              int code = 400)
    {
        DynamicJsonDocument doc(128);
        doc["error"] = message;
        sendJsonDocument(request, doc, code);
    }

    static int getIntParam(AsyncWebServerRequest *request,
                           const String &param,
                           int defaultValue)
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

    static bool getBoolParam(AsyncWebServerRequest *request,
                             const String &param,
                             bool defaultValue)
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

    static String getStringParam(AsyncWebServerRequest *request,
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

    static void printAvailableEndpoints()
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