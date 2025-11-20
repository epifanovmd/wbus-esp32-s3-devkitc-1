#ifndef WBUS_SNIFFER_H
#define WBUS_SNIFFER_H

#include <Arduino.h>

class WBusSniffer
{
private:
    String currentTx;
    void handleInfoCommands(String tx, String rx);
    void handleSensorsCommands(String tx, String rx);

public:
    void process();
};

extern WBusSniffer wBusSniffer;

#endif // WBUS_SNIFFER_H