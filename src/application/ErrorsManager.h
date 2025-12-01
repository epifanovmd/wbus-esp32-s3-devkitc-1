#pragma once
#include "../interfaces/IErrorsManager.h"
#include "../core/EventBus.h"
#include "../infrastructure/protocol/WBusErrorsDecoder.h"
#include "../infrastructure/protocol/WBusErrorDetailsDecoder.h"
#include "../infrastructure/protocol/WBusCommandBuilder.h"
#include "../application/CommandManager.h"

class ErrorsManager : public IErrorsManager
{
private:
  EventBus &eventBus;
  CommandManager &commandManager;

  WBusErrorsDecoder errorsDecoder;
  WBusErrorDetailsDecoder errorDetailsDecoder;
  ErrorCollection currentErrors;

public:
  ErrorsManager(EventBus &bus, CommandManager &cmdManager) : eventBus(bus), commandManager(cmdManager), errorDetailsDecoder(errorsDecoder) {}

  void checkErrors(bool loop = false, std::function<void(String, String, ErrorCollection *)> callback = nullptr) override
  {
    commandManager.addCommand(WBusCommandBuilder::createReadErrors(), [this, callback](String tx, String rx)
                              { handleCheckErrorsResponse(tx, rx, callback); }, loop);
  }

  void resetErrors(std::function<void(String, String)> callback = nullptr) override
  {
    commandManager.addCommand(WBusCommandBuilder::createClearErrors(),
                              [this, callback](String tx, String rx)
                              {
                                handleResetErrorsResponse(tx, rx, callback);
                              });
  }

  void readErrorDetails(uint8_t errorCode, std::function<void(String, String, ErrorDetails *)> callback = nullptr) override
  {
    String command = WBusCommandBuilder::createReadErrorDetails(errorCode);
    commandManager.addCommand(command,
                              [this, errorCode, callback](String tx, String rx)
                              {
                                handleErrorDetailsResponse(tx, rx, errorCode, callback);
                              });
  }

  // =========================================================================
  // ПУБЛИЧНЫЕ МЕТОДЫ ОБРАБОТКИ ОТВЕТОВ (для использования извне)
  // =========================================================================

  void handleCheckErrorsResponse(String tx, String rx, std::function<void(String, String, ErrorCollection *)> callback = nullptr)
  {
    if (!rx.isEmpty())
    {
      currentErrors = errorsDecoder.decodeErrorPacket(rx);
      eventBus.publish<ErrorCollection>(EventType::WBUS_ERRORS, currentErrors);

      if (callback)
      {
        callback(tx, rx, &currentErrors);
      }
    }
  }

  void handleResetErrorsResponse(String tx, String rx, std::function<void(String, String)> callback = nullptr)
  {
    if (!rx.isEmpty())
    {
      currentErrors.clear();
      eventBus.publish<ErrorCollection>(EventType::WBUS_ERRORS, currentErrors);
      eventBus.publish(EventType::WBUS_CLEAR_ERRORS_SUCCESS);

      if (callback)
      {
        callback(tx, rx);
      }
    }
    else
    {
      Serial.println("❌ Failed to clear errors");
      eventBus.publish(EventType::WBUS_CLEAR_ERRORS_FAILED);
    }
  }

  void handleErrorDetailsResponse(String tx, String rx, uint8_t errorCode, std::function<void(String, String, ErrorDetails *)> callback = nullptr)
  {
    ErrorDetails details = errorDetailsDecoder.decode(rx, errorCode);

    if (callback)
    {
      callback(tx, rx, &details);
    }
  }

  String getErrorsJson() const override
  {
    return currentErrors.toJson();
  }

  void printErrors() const override
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

  void clear() override
  {
    currentErrors.clear();
  }

  int getErrorCount() const override
  {
    return currentErrors.errorCount;
  }

  ErrorCollection getErrors() const override
  {
    return currentErrors;
  }

  void processNakResponse(const String &response) override
  {
    if (errorsDecoder.isNakResponse(response))
    {
      String cleanResponse = response;
      cleanResponse.replace(" ", "");

      if (cleanResponse.length() >= 10)
      {
        uint8_t failedCommand = Utils::hexStringToByte(cleanResponse.substring(8, 10));
        uint8_t errorCode = Utils::hexStringToByte(cleanResponse.substring(10, 12));

        Serial.println();
        Serial.println("❌ NAK получен:");
        errorsDecoder.decodeNakError(failedCommand, errorCode);
      }
    }
  }
};