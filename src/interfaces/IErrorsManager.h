#pragma once

class IErrorsManager
{
public:
    virtual ~IErrorsManager() = default;

    virtual void checkErrors(bool loop = false) = 0;
    virtual void resetErrors() = 0;
    virtual void readErrorDetails(uint8_t errorCode) = 0;

    virtual String getErrorsJson() const = 0;
    virtual void clear() = 0;
};