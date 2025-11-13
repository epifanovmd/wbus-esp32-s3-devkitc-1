// wbus-receiver.cpp

#include "wbus/wbus.h"

WBusReceivedData wBusReceivedData;

// Основная функция приема и обработки пакетов
void readWBusData()
{
  wBusReceivedData.resetState(); // Сбрасываем состояние при каждом вызове

  while (WBusSerial.available())
  {
    byte readByte = WBusSerial.read();

    if (!wBusReceivedData.isReceiving())
    {
      // Ищем начало пакета
      if (readByte == RXHEADER)
      {
        wBusReceivedData.startRxReception(readByte);
      }
      else if (readByte == TXHEADER)
      {
        wBusReceivedData.startTxReception(readByte);
      }
    }
    else
    {
      // Прием продолжается
      wBusReceivedData.addByte(readByte);

      // Проверка завершения
      if (wBusReceivedData.isPacketComplete())
      {
        if (wBusReceivedData.isReceivingRx)
        {
          wBusReceivedData.completeRxReception();
          wBusReceivedData.printRx();
        }
        if (wBusReceivedData.isReceivingTx)
        {
          wBusReceivedData.completeTxReception();
          wBusReceivedData.printTx();
        }
        // Автоматический сброс после завершения
        wBusReceivedData.bytesToRead = 0;
        wBusReceivedData.bytesRead = 0;
      }
    }
  }
}