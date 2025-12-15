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
  ErrorsManager(EventBus &bus, CommandManager &cmdManager) : eventBus(bus),
                                                             commandManager(cmdManager) {}

  void checkErrors(bool loop = false) override
  {
    commandManager.addPriorityCommand(WBusCommandBuilder::createReadErrors(), loop);
  }

  void resetErrors() override
  {
    commandManager.addPriorityCommand(WBusCommandBuilder::createClearErrors(), false, [this](String tx, String rx)
                                      { commandManager.addPriorityCommand(WBusCommandBuilder::createReadErrors()); });
  }

  void readErrorDetails(uint8_t errorCode) override
  {
    commandManager.addPriorityCommand(WBusCommandBuilder::createReadErrorDetails(errorCode));
  }

  // =========================================================================

  void handleCheckErrorsResponse(String tx, String rx, bool needReadDetails = false)
  {
    currentErrors = errorsDecoder.decodeErrorPacket(rx);
    eventBus.publish<ErrorCollection>(EventType::WBUS_ERRORS, currentErrors);

    if (needReadDetails)
    {
      for (size_t i = 0; i < currentErrors.errorCount; i++)
      {

        WebastoError error = currentErrors.errors[i];

        if (error.code)
        {
          readErrorDetails(error.code);
        }
      }
    }
  }

  void handleResetErrorsResponse(String tx, String rx)
  {
    currentErrors.clear();
    eventBus.publish<ErrorCollection>(EventType::WBUS_ERRORS, currentErrors);
  }

  void handleErrorDetailsResponse(String tx, String rx, uint8_t errorCode, std::function<void(String, String, ErrorDetails *)> callback = nullptr)
  {
    ErrorDetails details = errorDetailsDecoder.decode(rx, errorCode);
    eventBus.publish<ErrorDetails>(EventType::WBUS_DETAILS_ERROR, details);
  }

  String getErrorsJson() const override
  {
    return currentErrors.toJson();
  }

  void clear() override
  {
    currentErrors.clear();
  }
};