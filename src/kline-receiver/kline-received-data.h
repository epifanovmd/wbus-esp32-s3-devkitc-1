#ifndef KLINERECEIVEDDATA_H
#define KLINERECEIVEDDATA_H

#include <Arduino.h>

enum KLineReceptionStates
{
    KLINE_IDLE,
    KLINE_RX_RECEIVED,
    KLINE_TX_RECEIVED
};

struct KLineReceivedData
{
    String rxString = "";
    String txString = "";
    bool isReceivingRx = false;
    bool isReceivingTx = false;
    int bytesToRead = 0;
    int bytesRead = 0;
    KLineReceptionStates rx_reception_state = KLINE_IDLE;
    KLineReceptionStates tx_reception_state = KLINE_IDLE;

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
    bool isRxReceived() const { return rx_reception_state == KLINE_RX_RECEIVED; }
    bool isTxReceived() const { return tx_reception_state == KLINE_TX_RECEIVED; }

    // Методы для получения данных
    String getRxData() const { return rxString; }
    String getTxData() const { return txString; }
};

extern KLineReceivedData kLineReceivedData;

#endif // KLINERECEIVEDDATA_H