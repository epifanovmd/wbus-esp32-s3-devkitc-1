#ifndef WBUS_ERRORS_H
#define WBUS_ERRORS_H

#include <Arduino.h>
#include "wbus-errors-decoder.h"

class WebastoErrors
{
private:
    ErrorCollection currentErrors;

    void handleErrorResponse(String tx, String rx);

public:
    void check(bool loop = false, std::function<void(String, String)> callback = nullptr);
    void clear();
    void printErrors();

    void stopLoop();
    bool hasErrors() const { return currentErrors.hasErrors; }
    int errorCount() const { return currentErrors.errorCount; }
    ErrorCollection getErrors() const { return currentErrors; }
};

extern WebastoErrors webastoErrors;

#endif // WBUS_ERRORS_H