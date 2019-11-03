/////////////////////////////////////////////////////////////////////////////
/** @file
Web Server

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED__WEB_SERVER
#define INCLUDED__WEB_SERVER

//- includes
#include <ArduinoJson.h>
#include <ESPAsyncWebServer.h>

//- forwards
class Settings;
class WifiManager;

/////////////////////////////////////////////////////////////////////////////
/// web server
/// https://github.com/me-no-dev/ESPAsyncWebServer
class WebServer {
public:
    explicit WebServer(Settings& settings);
    WebServer(const WebServer&) = delete;
    WebServer& operator=(const WebServer&) = delete;

    void begin(WifiManager& wifi);
    void tick();

private:
    void onWebSocketEvent_(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len);
    void onJsonRpc_(AsyncWebSocketClient* client, char* data, size_t len);

    AsyncWebServer  server_{80};        ///< async web server
    AsyncWebSocket  serverWebSocket_;   ///< async web socket
    Settings&       settings_;          ///< settings access
    AsyncWebServerRequest* update_request_{nullptr};    ///< tracks update request
    int             last_update_percent_{-1};           ///< last reported percentage
};

#endif // INCLUDED__WEB_SERVER
