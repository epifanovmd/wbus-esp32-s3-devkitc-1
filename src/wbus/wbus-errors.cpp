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

    if (_currentErrors.isEmpty())
    {
        Serial.println("✅ Ошибок не обнаружено");
    }
    else
    {
        Serial.println("📋 Найдено ошибок: " + String(_currentErrors.errorCount));
        Serial.println();

        for (size_t i = 0; i < _currentErrors.errors.size(); i++)
        {
            const WebastoError &error = _currentErrors.errors[i];
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

void WebastoErrors::handleErrorResponse(bool status, String tx, String rx)
{
    if (!status)
    {
        return;
    }

    // Декодируем и заполняем структуру
    _currentErrors = webastoErrorsDecoder.decodeErrorPacket(rx);

    // Автоматически выводим ошибки
    printErrors();
}

// =============================================================================
// ОСНОВНЫЕ МЕТОДЫ
// =============================================================================

void WebastoErrors::check(bool loop)
{
    wbusQueue.add(CMD_READ_ERRORS_LIST, [this](bool status, String tx, String rx)
                  { this->handleErrorResponse(status, tx, rx); }, loop);
}

void WebastoErrors::clear()
{
    wbusQueue.add(CMD_CLEAR_ERRORS, [this](bool success, String tx, String rx)
                  {
    this->_currentErrors.clear();

    // Выводим подтверждение
    Serial.println();
    Serial.println("✅ Ошибки очищены");
    Serial.println(); });
}

void WebastoErrors::stopLoop()
{
    wbusQueue.removeCommand(CMD_READ_ERRORS_LIST);
    Serial.println("⏹️ Циклическая проверка ошибок остановлена");
}