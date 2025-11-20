#include "wbus-errors.h"
#include "common/utils/utils.h"
#include <functional>
#include "wbus-queue.h"
#include "wbus.constants.h"
#include <ArduinoJson.h>

WebastoErrors webastoErrors;

ErrorCollection *WebastoErrors::handleErrorResponse(String rx)
{
    if (!rx.isEmpty())
    {
        // Декодируем и заполняем структуру
        currentErrors = webastoErrorsDecoder.decodeErrorPacket(rx);

        return &currentErrors;
    }

    return nullptr;
}

void WebastoErrors::check(bool loop, std::function<void(String, String, ErrorCollection *errors)> callback)
{
    wbusQueue.add(CMD_READ_ERRORS_LIST, [this, callback](String tx, String rx)
                  {
    ErrorCollection* errors = this -> handleErrorResponse(rx);
    if (callback != nullptr) {
      callback(tx, rx, errors);
    } }, loop);
}

void WebastoErrors::reset()
{
    wbusQueue.add(CMD_CLEAR_ERRORS, [this](String tx, String rx)
                  {
                      if (!rx.isEmpty())
                      {
                          this->currentErrors.clear();
                      }

                      // Выводим подтверждение
                      // Serial.println();
                      // Serial.println("✅ Ошибки очищены");
                      // Serial.println();
                  });
}

void WebastoErrors::clear()
{
    currentErrors.clear();
}

void WebastoErrors::stopLoop()
{
    wbusQueue.removeCommand(CMD_READ_ERRORS_LIST);
    Serial.println("⏹️ Циклическая проверка ошибок остановлена");
}

// =============================================================================
// ФУНКЦИЯ ФОРМИРОВАНИЯ JSON БЕЗ КЛАССИФИКАЦИИ
// =============================================================================

String WebastoErrors::createJsonErrors(const ErrorCollection &data)
{
    DynamicJsonDocument doc(4096);

    doc["count"] = data.errorCount;

    JsonArray errorArray = doc.createNestedArray("errors");

    if (!data.isEmpty())
    {
        for (const WebastoError &error : data.errors)
        {
            JsonObject errorObj = errorArray.createNestedObject();
            errorObj["code"] = error.code;
            errorObj["hex_code"] = error.hexCode;
            errorObj["description"] = error.description;
            errorObj["counter"] = error.counter;
        }
    }

    String json;
    serializeJson(doc, json);
    return json;
}

String WebastoErrors::createJsonErrors()
{
    return createJsonErrors(currentErrors);
}

// =============================================================================
// ФУНКЦИЯ ВЫВОДА ОШИБОК (ПЕРЕНЕСЕНА СЮДА)
// =============================================================================

void WebastoErrors::printErrors()
{
    Serial.println();
    Serial.println("═══════════════════════════════════════════════════════════");
    Serial.println("               🚨 ОШИБКИ WEBASTO                          ");
    Serial.println("═══════════════════════════════════════════════════════════");

    if (currentErrors.isEmpty())
    {
        Serial.println("✅ Ошибок не обнаружено");
    }
    else
    {
        Serial.println("📋 Найдено ошибок: " + String(currentErrors.errorCount));
        Serial.println();

        for (size_t i = 0; i < currentErrors.errors.size(); i++)
        {
            const WebastoError &error = currentErrors.errors[i];
            Serial.print("   ");
            Serial.print(i + 1);
            Serial.print(". ");
            Serial.print(error.hexCode);
            Serial.print(" (");
            Serial.print(error.code, DEC);
            Serial.print(") - ");
            Serial.print(error.description);

            if (error.counter > 0)
            {
                Serial.print(" [Счетчик: ");
                Serial.print(error.counter);
                Serial.print("]");
            }
            Serial.println();
        }
    }

    Serial.println("═══════════════════════════════════════════════════════════");
    Serial.println();
}