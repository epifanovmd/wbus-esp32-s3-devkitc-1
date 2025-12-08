// src/infrastructure/network/ConfigApiHandlers.h
#pragma once
#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <LittleFS.h>
#include "../../core/ConfigManager.h"
#include "../../core/FileSystemManager.h"
#include "./ApiHelpers.h"

class ConfigApiHandlers
{
private:
  AsyncWebServer &server;
  ConfigManager &configManager;
  FileSystemManager &fsManager;

public:
  ConfigApiHandlers(AsyncWebServer &serv, ConfigManager &configMngr, FileSystemManager &fsMgr) : server(serv),
                                                                                                 configManager(configMngr),
                                                                                                 fsManager(fsMgr) {}

  void setupEndpoints()
  {
    // Получить текущую конфигурацию
    server.on("/api/config", HTTP_GET, [this](AsyncWebServerRequest *request)
              {
      if (!configManager.isConfigLoaded()) {
        ApiHelpers::sendJsonError(request, "Config not loaded", 503);
        return;
      }

      String json = configManager.getConfigJson();
      ApiHelpers::sendJsonResponse(request, json); });

    // Обновить конфигурацию
    server.addHandler(new AsyncCallbackJsonWebHandler(
        "/api/config",
        [this](AsyncWebServerRequest *request, JsonVariant &json)
        {
          if (!json.is<JsonObject>())
          {
            ApiHelpers::sendJsonError(request, "Invalid JSON", 400);
            return;
          }

          JsonObject newConfig = json.as<JsonObject>();

          ConfigUpdateResult result = configManager.updateConfig(newConfig);
          handleUpdateConfig(request, result);
        }));

    // Сбросить конфигурацию к значениям по умолчанию
    server.on("/api/config/reset", HTTP_POST, [this](AsyncWebServerRequest *request)
              {
      // Используем метод ConfigManager
      ConfigUpdateResult result = configManager.resetToDefaults();

         handleUpdateConfig(request, result); });

    // Скачать конфиг файл
    server.on("/api/config/download", HTTP_GET, [this](AsyncWebServerRequest *request)
              {
      if (fsManager.exists("/config.json")) {
        // Правильный способ отправить файл из LittleFS
        request -> send(LittleFS, "/config.json", "application/json", false);
      } else {
        ApiHelpers::sendJsonError(request, "Config file not found", 404);
      } });
  }

private:
  void handleUpdateConfig(AsyncWebServerRequest *request, ConfigUpdateResult &result)
  {
    DynamicJsonDocument responseDoc(256);

    if (result == ConfigUpdateResult::SUCCESS || result == ConfigUpdateResult::SUCCESS_RESTART_REQUIRED)
    {
      String json = configManager.getConfigJson();
      ApiHelpers::sendJsonResponse(request, json);
    }
    else
    {
      ApiHelpers::sendJsonError(request, "Failed to save configuration", 500);
    }
  }
};