#ifndef WBUS_ERRORS_H
#define WBUS_ERRORS_H

#include <Arduino.h>
#include "wbus-errors-decoder.h"

class WebastoErrors
{
private:
    ErrorCollection currentErrors;

public:
    ErrorCollection* handleErrorResponse(String rx);

    void check(bool loop = false, std::function<void(String, String, ErrorCollection*)> callback = nullptr);
    void reset();
    void clear();

    // Функция формирования JSON без классификации
    String createJsonErrors(const ErrorCollection& errors);
    String createJsonErrors(); // Перегруженная версия с текущими ошибками

    void printErrors();
    void stopLoop();
    int errorCount() const { return currentErrors.errorCount; }
    ErrorCollection getErrors() const { return currentErrors; }
};

extern WebastoErrors webastoErrors;

#endif // WBUS_ERRORS_H