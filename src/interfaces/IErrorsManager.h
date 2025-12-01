#pragma once
#include "../domain/Entities.h"

class CommandManager;

class IErrorsManager
{
public:
    virtual ~IErrorsManager() = default;

    virtual void checkErrors(bool loop = false, std::function<void(String, String, ErrorCollection *)> callback = nullptr) = 0;
    virtual void resetErrors(std::function<void(String, String)> callback = nullptr) = 0;
    virtual void readErrorDetails(uint8_t errorCode, std::function<void(String, String, ErrorDetails *)> callback = nullptr) = 0;

    virtual String getErrorsJson() const = 0;
    virtual void clear() = 0;
};