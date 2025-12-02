// src/interfaces/IWebServer.h
#pragma once
#include <functional>

class IWebServer {
public:
    virtual ~IWebServer() = default;
    
    virtual bool initialize() = 0;
    virtual void process() = 0;  // Может быть пустым для async
    
    virtual void on(const String& uri, 
                   std::function<void(AsyncWebServerRequest* request)> handler) = 0;
    virtual void on(const String& uri, WebRequestMethod method,
                   std::function<void(AsyncWebServerRequest* request)> handler) = 0;
};