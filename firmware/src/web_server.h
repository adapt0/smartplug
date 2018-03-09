/////////////////////////////////////////////////////////////////////////////
/** @file
Web Server

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED__WEB_SERVER
#define INCLUDED__WEB_SERVER

//- includes
#include <ESPAsyncWebServer.h>

//- forwards
class Settings;

/////////////////////////////////////////////////////////////////////////////
/// web server
/// https://github.com/me-no-dev/ESPAsyncWebServer
class WebServer {
public:
    explicit WebServer(Settings& settings);
    void begin();
    void tick();

private:
    void onWebSocketEvent_(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len);

    AsyncWebServer  server_{80};    ///< async web server
    Settings&       settings_;      ///< settings access
};

#endif // INCLUDED__WEB_SERVER
