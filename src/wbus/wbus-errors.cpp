#include "wbus-errors.h"
#include "common/utils/utils.h"
#include <functional>
#include "wbus-queue.h"
#include "wbus.constants.h"

WebastoErrors webastoErrors;

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

// =============================================================================
// ОБРАБОТЧИК ОТВЕТА (ОБНОВЛЕННЫЙ)
// =============================================================================

void WebastoErrors::handleErrorResponse(String tx, String rx)
{
    if (!rx.isEmpty())
    {
        // Декодируем и заполняем структуру
        currentErrors = webastoErrorsDecoder.decodeErrorPacket(rx);
    }
}

void WebastoErrors::handleCommandResponse(String tx, String rx)
{
    if (rx.isEmpty())
        return; // Не обрабатываем пустые ответы

    if (tx == CMD_READ_ERRORS_LIST)
        handleErrorResponse(tx, rx);
    // else
    //     Serial.println("❌ Для этой команды нет обработчика: " + tx);
}

// =============================================================================
// ОСНОВНЫЕ МЕТОДЫ
// =============================================================================

void WebastoErrors::check(bool loop, std::function<void(String, String)> callback)
{
    wbusQueue.add(CMD_READ_ERRORS_LIST, [this, callback](String tx, String rx)
                  {
    this -> handleCommandResponse(tx, rx);
    if (callback != nullptr) {
      callback(tx, rx);
    } }, loop);
}

void WebastoErrors::clear()
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

void WebastoErrors::stopLoop()
{
    wbusQueue.removeCommand(CMD_READ_ERRORS_LIST);
    Serial.println("⏹️ Циклическая проверка ошибок остановлена");
}