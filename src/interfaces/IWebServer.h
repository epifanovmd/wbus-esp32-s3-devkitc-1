// src/interfaces/IWebServer.h
#pragma once
#include <functional>

class IWebServer {
public:
    virtual ~IWebServer() = default;
    
    virtual bool initialize() = 0;
    virtual void process() = 0;
};