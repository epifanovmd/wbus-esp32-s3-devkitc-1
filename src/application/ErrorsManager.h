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
                                                             commandManager(cmdManager),
                                                             errorDetailsDecoder(errorsDecoder) {}

  void checkErrors(bool loop = false, std::function<void(String, String, ErrorCollection *)> callback = nullptr) override
  {
    commandManager.addCommand(WBusCommandBuilder::createReadErrors(), [this, callback](String tx, String rx)
                              { handleCheckErrorsResponse(tx, rx, callback); }, loop);
  }

  void resetErrors(std::function<void(String, String)> callback = nullptr) override
  {
    commandManager.addPriorityCommand(WBusCommandBuilder::createClearErrors(), [this, callback](String tx, String rx)
                                      {
      commandManager.addCommand(WBusCommandBuilder::createReadErrors(), [this, callback](String tx, String rx) {
        handleCheckErrorsResponse(tx, rx);
      });
      handleResetErrorsResponse(tx, rx, callback); });
  }

  void readErrorDetails(uint8_t errorCode, std::function<void(String, String, ErrorDetails *)> callback = nullptr) override
  {
    commandManager.addCommand(WBusCommandBuilder::createReadErrorDetails(errorCode), [this, errorCode, callback](String tx, String rx)
                              { handleErrorDetailsResponse(tx, rx, errorCode, callback); });
  }

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

  void clear() override
  {
    currentErrors.clear();
  }
};