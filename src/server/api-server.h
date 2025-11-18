#ifndef API_SERVER_H
#define API_SERVER_H

#include <Arduino.h>
#include <WebServer.h>

class ApiServer
{
private:
    WebServer server;

public:
    ApiServer();

    void begin();
    void loop();

private:
    void handleRoot();
    void handleNotFound();
    String getHTMLPage();
};

extern ApiServer apiServer;

#endif //API_SERVER_H