// constants.h

#ifndef CONSTANTS_H
#define CONSTANTS_H

// Пины для управления TJA1020
#define NSLP_PIN 7     // Sleep control (active LOW)
#define NWAKE_PIN 6    // Wake-up input (active LOW) 
#define RXD_PULLUP 8   // Пин для подтяжки RXD (open-drain)

// Конфигурация Webasto W-Bus
#define TXHEADER 0xF4  // WTT -> Нагреватель
#define RXHEADER 0x4F  // Нагреватель -> WTT
#define MESSAGE_BUFFER_SIZE 64
#define BAUD_RATE 2400
#define SERIAL_CONFIG SERIAL_8E1  // 8 бит, Even parity, 1 стоп-бит
#define RX_TJA_PIN 18
#define TX_TJA_PIN 17

#endif // CONSTANTS_H