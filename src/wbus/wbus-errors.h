#ifndef WBUS_ERRORS_H
#define WBUS_ERRORS_H

#include <Arduino.h>
#include "wbus-errors-decoder.h"

class WebastoErrors
{
private:
    ErrorCollection _currentErrors;

    void handleErrorResponse(bool status, String tx, String rx);
    void printErrors();

public:
    void check(bool loop = false);
    void clear();

    void stopLoop();
    bool hasErrors() const { return _currentErrors.hasErrors; }
    int errorCount() const { return _currentErrors.errorCount; }
    ErrorCollection getErrors() const { return _currentErrors; }
};

extern WebastoErrors webastoErrors;

#endif // WBUS_ERRORS_H