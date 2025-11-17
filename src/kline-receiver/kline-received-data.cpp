#include "kline-received-data.h"
#include "common/utils/utils.h"

void KLineReceivedData::startRxReception(byte headerByte)
{
    isReceivingRx = true;
    rxString = "";
    bytesRead = 1;
    rxString += byteToHexString(headerByte) + " ";
}

void KLineReceivedData::startTxReception(byte headerByte)
{
    isReceivingTx = true;
    txString = "";
    bytesRead = 1;
    txString += byteToHexString(headerByte) + " ";
}

void KLineReceivedData::addByte(byte readByte)
{
    // Второй байт - длина пакета
    if (bytesRead == 1)
    {
        bytesToRead = readByte;
    }

    String hexByte = byteToHexString(readByte);

    if (isReceivingRx)
        rxString += hexByte + " ";
    if (isReceivingTx)
        txString += hexByte + " ";
    bytesRead++;
}

bool KLineReceivedData::isPacketComplete()
{
    return bytesRead >= bytesToRead + 2;
}

void KLineReceivedData::completeRxReception()
{
    rxString.trim();
    isReceivingRx = false;
    rx_reception_state = KLINE_RX_RECEIVED;
}

void KLineReceivedData::completeTxReception()
{
    txString.trim();
    isReceivingTx = false;
    tx_reception_state = KLINE_TX_RECEIVED;
}

void KLineReceivedData::printTx()
{
    Serial.println();
    Serial.print("📤 TX: " + getTxData());
}

void KLineReceivedData::printRx()
{

    Serial.println();
    Serial.print("📨 RX: " + getRxData());
}

void KLineReceivedData::reset()
{
    resetRx();
    resetTx();
}

void KLineReceivedData::resetState()
{
    rx_reception_state = KLINE_IDLE;
    tx_reception_state = KLINE_IDLE;
}

void KLineReceivedData::resetRx()
{
    rxString = "";
    isReceivingRx = false;
    rx_reception_state = KLINE_IDLE;
}

void KLineReceivedData::resetTx()
{
    txString = "";
    isReceivingTx = false;
    tx_reception_state = KLINE_IDLE;
}