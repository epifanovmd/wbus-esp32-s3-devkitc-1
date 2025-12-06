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
        // Serial.println();
        // Serial.println("🌐 Available API Endpoints:");
        // Serial.println();
    }
};