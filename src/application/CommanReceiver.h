#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include "../common/Constants.h"
#include "../common/Utils.h"
#include "../core/EventBus.h"

enum class KLineReceptionStates {
    IDLE,
    RX_RECEIVED,
    TX_RECEIVED
};

struct KLineReceivedData {
    String rxString = "";
    String txString = "";
    bool isReceivingRx = false;
    bool isReceivingTx = false;
    int bytesToRead = 0;
    int bytesRead = 0;
    KLineReceptionStates rx_reception_state = KLineReceptionStates::IDLE;
    KLineReceptionStates tx_reception_state = KLineReceptionStates::IDLE;

    void startRxReception(byte headerByte) {
        isReceivingRx = true;
        rxString = "";
        bytesRead = 1;
        rxString += Utils::byteToHexString(headerByte) + " ";
    }

    void startTxReception(byte headerByte) {
        isReceivingTx = true;
        txString = "";
        bytesRead = 1;
        txString += Utils::byteToHexString(headerByte) + " ";
    }

    void addByte(byte readByte) {
        if (bytesRead == 1) {
            bytesToRead = readByte;
        }

        String hexByte = Utils::byteToHexString(readByte);

        if (isReceivingRx) rxString += hexByte + " ";
        if (isReceivingTx) txString += hexByte + " ";
        bytesRead++;
    }

    bool isPacketComplete() {
        return bytesRead >= bytesToRead + 2;
    }

    void completeRxReception() {
        rxString.trim();
        isReceivingRx = false;
        rx_reception_state = KLineReceptionStates::RX_RECEIVED;
    }

    void completeTxReception() {
        txString.trim();
        isReceivingTx = false;
        tx_reception_state = KLineReceptionStates::TX_RECEIVED;
    }

    void reset() {
        rxString = "";
        txString = "";
        isReceivingRx = false;
        isReceivingTx = false;
        bytesToRead = 0;
        bytesRead = 0;
        rx_reception_state = KLineReceptionStates::IDLE;
        tx_reception_state = KLineReceptionStates::IDLE;
    }

    bool isReceiving() const { return isReceivingRx || isReceivingTx; }
    bool isRxReceived() const { return rx_reception_state == KLineReceptionStates::RX_RECEIVED; }
    bool isTxReceived() const { return tx_reception_state == KLineReceptionStates::TX_RECEIVED; }
    String getRxData() const { return rxString; }
    String getTxData() const { return txString; }
};

class CommanReceiver {
private:
    HardwareSerial& serial;
    EventBus& eventBus;
    KLineReceivedData receivedData;
    String currentTx;

public:
    CommanReceiver(HardwareSerial& serialRef, EventBus& bus) 
        : serial(serialRef), eventBus(bus) {}

    void process() {
        receivedData.reset();

        while (serial.available()) {
            byte readByte = serial.read();

            Serial.println(readByte);

            if (!receivedData.isReceiving()) {
                if (readByte == RXHEADER) {
                    receivedData.startRxReception(readByte);
                } else if (readByte == TXHEADER) {
                    receivedData.startTxReception(readByte);
                    currentTx = ""; // Сбрасываем текущую передачу
                }
            } else {
                receivedData.addByte(readByte);

                if (receivedData.isPacketComplete()) {
                    if (receivedData.isReceivingRx) {
                        receivedData.completeRxReception();
                        handleRxPacket(getRxData());
                        Serial.print("📤 TX: " + getTxData());
                    }
                    if (receivedData.isReceivingTx) {
                        receivedData.completeTxReception();
                        currentTx = getTxData();
                        Serial.print("📨 RX: " + getRxData());
                    }
                    receivedData.bytesToRead = 0;
                    receivedData.bytesRead = 0;
                }
            }
        }
    }

    bool isRxReceived() const { return receivedData.isRxReceived(); }
    bool isTxReceived() const { return receivedData.isTxReceived(); }
    String getRxData() const { return receivedData.getRxData(); }
    String getTxData() const { return receivedData.getTxData(); }
    String getCurrentTx() const { return currentTx; }

private:
    void handleRxPacket(const String& rxData) {
        // Здесь будет логика обработки пакетов как в WBusSniffer
        eventBus.publish(EventType::COMMAND_RECEIVED, "WBusReceiver");
        
        // Отправляем в WebSocket если нужно
        DynamicJsonDocument doc(512);
        doc["type"] = "rx";
        doc["data"] = rxData;
        
        String json;
        serializeJson(doc, json);
        eventBus.publish(EventType::WEBSOCKET_MESSAGE, "WBusReceiver");
    }
};