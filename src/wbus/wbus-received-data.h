#ifndef WBUSRECEIVEDDATA_H
#define WBUSRECEIVEDDATA_H

#include <Arduino.h>

enum WbusReceptionStates
{
    WBUS_RX_IDLE,
    WBUS_RX_RECEIVED
};

struct WBusReceivedData
{
    String rxString = "";
    String txString = "";
    bool isReceivingRx = false;
    bool isReceivingTx = false;
    int bytesToRead = 0;
    int bytesRead = 0;
    WbusReceptionStates rx_reception_state = WBUS_RX_IDLE;
    WbusReceptionStates tx_reception_state = WBUS_RX_IDLE;

    // Методы для инициализации приема
    void startRxReception(byte headerByte);
    void startTxReception(byte headerByte);

    // Методы для добавления байтов
    void addByte(byte readByte);

    // Методы для проверки завершения
    bool isPacketComplete();

    // Методы для завершения приема
    void completeRxReception();
    void completeTxReception();

    // Методы для сброса состояния
    void reset();
    void resetState();
    void resetRx();
    void resetTx();
    void printTx();
    void printRx();

    // Методы для проверки состояния
    bool isReceiving() const { return isReceivingRx || isReceivingTx; }
    bool isRxReceived() const { return rx_reception_state == WBUS_RX_RECEIVED; }
    bool isTxReceived() const { return tx_reception_state == WBUS_RX_RECEIVED; }

    // Методы для получения данных
    String getRxData() const { return rxString; }
    String getTxData() const { return txString; }

    // Вспомогательные методы
    String byteToHexString(byte b) const;
};

extern WBusReceivedData wBusReceivedData;

#endif // WBUSRECEIVEDDATA_H