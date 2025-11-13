#include "wbus/wbus-received-data.h"

void WBusReceivedData::startRxReception(byte headerByte)
{
    isReceivingRx = true;
    rxString = "";
    bytesRead = 1;
    rxString += byteToHexString(headerByte) + " ";
}

void WBusReceivedData::startTxReception(byte headerByte)
{
    isReceivingTx = true;
    txString = "";
    bytesRead = 1;
    txString += byteToHexString(headerByte) + " ";
}

void WBusReceivedData::addByte(byte readByte)
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

bool WBusReceivedData::isPacketComplete()
{
    return bytesRead >= bytesToRead + 2;
}

void WBusReceivedData::completeRxReception()
{
    rxString.trim();
    isReceivingRx = false;
    rx_reception_state = WBUS_RX_RECEIVED;
}

void WBusReceivedData::completeTxReception()
{
    txString.trim();
    isReceivingTx = false;
    tx_reception_state = WBUS_RX_RECEIVED;
}

void WBusReceivedData::printTx()
{
    Serial.println();
    Serial.print("📤 TX: " + getTxData());
}

void WBusReceivedData::printRx()
{

    Serial.println();
    Serial.print("📨 RX: " + getRxData());
}

void WBusReceivedData::reset()
{
    resetRx();
    resetTx();
}

void WBusReceivedData::resetState()
{
    rx_reception_state = WBUS_RX_IDLE;
    tx_reception_state = WBUS_RX_IDLE;
}

void WBusReceivedData::resetRx()
{
    rxString = "";
    isReceivingRx = false;
    rx_reception_state = WBUS_RX_IDLE;
}

void WBusReceivedData::resetTx()
{
    txString = "";
    isReceivingTx = false;
    tx_reception_state = WBUS_RX_IDLE;
}

String WBusReceivedData::byteToHexString(byte b) const
{
    return (b < 0x10) ? "0" + String(b, HEX) : String(b, HEX);
}