// wbus-receiver.cpp

#include "kline-receiver.h"

KLineReceiver kLineReceiver;

// Основная функция приема и обработки пакетов
void KLineReceiver::process()
{
  kLineReceivedData.resetState(); // Сбрасываем состояние при каждом вызове

  while (KLineSerial.available())
  {
    byte readByte = KLineSerial.read();

    if (!kLineReceivedData.isReceiving())
    {
      // Ищем начало пакета
      if (readByte == RXHEADER)
      {
        kLineReceivedData.startRxReception(readByte);
      }
      else if (readByte == TXHEADER)
      {
        kLineReceivedData.startTxReception(readByte);
      }
    }
    else
    {
      // Прием продолжается
      kLineReceivedData.addByte(readByte);

      // Проверка завершения
      if (kLineReceivedData.isPacketComplete())
      {
        if (kLineReceivedData.isReceivingRx)
        {
          kLineReceivedData.completeRxReception();
        }
        if (kLineReceivedData.isReceivingTx)
        {
          kLineReceivedData.completeTxReception();
        }
        // Автоматический сброс после завершения
        kLineReceivedData.bytesToRead = 0;
        kLineReceivedData.bytesRead = 0;
      }
    }
  }
}