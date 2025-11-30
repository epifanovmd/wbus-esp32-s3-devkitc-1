// src/application/ErrorsManager.h
#pragma once
#include "../interfaces/IErrorsManager.h"
#include "../core/EventBus.h"
#include "../infrastructure/protocol/WBusErrorsDecoder.h"
#include "../infrastructure/protocol/WBusCommandBuilder.h"
#include "../application/CommandManager.h"

class ErrorsManager: public IErrorsManager {
  private: EventBus & eventBus;
  CommandManager & commandManager;

  WBusErrorsDecoder errorsDecoder;
  ErrorCollection currentErrors;

  public: ErrorsManager(EventBus & bus, CommandManager & cmdManager): eventBus(bus),
  commandManager(cmdManager) {}

  void checkErrors(bool loop = false, std:: function < void(String, String, ErrorCollection * ) > callback = nullptr) override {
    commandManager.addCommand(WBusCommandBuilder::createReadErrors(),
      [this, loop, callback](String tx, String rx) {
        if (!rx.isEmpty()) {
          currentErrors = errorsDecoder.decodeErrorPacket(rx);
          // printErrors();

          eventBus.publish < ErrorCollection > (EventType::WBUS_ERRORS, currentErrors);

          if (callback) {
            callback(tx, rx, & currentErrors);
          }

        }
      }, loop);
  }

  void resetErrors(std:: function < void(String, String) > callback = nullptr) override {
    commandManager.addCommand(WBusCommandBuilder::createReadErrors(),
      [this, callback](String tx, String rx) {
        if (!rx.isEmpty()) {
          currentErrors.clear();
        //   Serial.println("✅ Errors cleared successfully");
          eventBus.publish < ErrorCollection > (EventType::WBUS_ERRORS, currentErrors);
          eventBus.publish(EventType::WBUS_CLEAR_ERRORS_SUCCESS);

          if (callback) {
            callback(tx, rx);
          }
        } else {
          Serial.println("❌ Failed to clear errors");
          eventBus.publish(EventType::WBUS_CLEAR_ERRORS_FAILED);
        }
      });
  }

  String getErrorsJson() const {
    return currentErrors.toJson();
  }

  void printErrors() const {
    Serial.println();
    Serial.println("═══════════════════════════════════════════════════════════");
    Serial.println("               🚨 ОШИБКИ WEBASTO                          ");
    Serial.println("═══════════════════════════════════════════════════════════");

    if (currentErrors.isEmpty()) {
      Serial.println("✅ Ошибок не обнаружено");
    } else {
      Serial.println("📋 Найдено ошибок: " + String(currentErrors.errorCount));
      Serial.println();

      for (size_t i = 0; i < currentErrors.errors.size(); i++) {
        const WebastoError & error = currentErrors.errors[i];
        Serial.print("   ");
        Serial.print(i + 1);
        Serial.print(". ");
        Serial.print(error.hexCode);
        Serial.print(" (");
        Serial.print(error.code, DEC);
        Serial.print(") - ");
        Serial.print(error.description);

        if (error.counter > 0) {
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

  void clear() {
    currentErrors.clear();
  }

  int getErrorCount() const override {
    return currentErrors.errorCount;
  }

  ErrorCollection getErrors() const override {
    return currentErrors;
  }

  void processNakResponse(const String & response) {
    if (errorsDecoder.isNakResponse(response)) {
      String cleanResponse = response;
      cleanResponse.replace(" ", "");

      if (cleanResponse.length() >= 10) {
        uint8_t failedCommand = Utils::hexStringToByte(cleanResponse.substring(8, 10));
        uint8_t errorCode = Utils::hexStringToByte(cleanResponse.substring(10, 12));

        Serial.println();
        Serial.println("❌ NAK получен:");
        errorsDecoder.decodeNakError(failedCommand, errorCode);
      }
    }
  }
};