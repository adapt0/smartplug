/////////////////////////////////////////////////////////////////////////////
/** @file
Web Server

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////

//- includes
#include "web_server.h"
#include "settings.h"
#include "web_server_asset_handler.h"
#include <ArduinoJson.h>

/////////////////////////////////////////////////////////////////////////////
/// constructor
WebServer::WebServer(Settings& settings)
: settings_(settings)
{ }

/////////////////////////////////////////////////////////////////////////////
/// begin web server
void WebServer::begin() {
    // API requests
    {
        server_.on("/api/v1/ping", HTTP_GET, [](AsyncWebServerRequest* request) {
            request->send(200, "text/plain", "pong");
        });
        server_.on("/api/v1/state", HTTP_GET, [this](AsyncWebServerRequest* request) {
            auto* response = request->beginResponseStream("application/json");
            if (response) {
                DynamicJsonBuffer buffer;
                settings_.toJson(buffer).printTo(*response);
                request->send(response);
            }
        });
        server_.on("/api", [](AsyncWebServerRequest* request) {
            request->send(404);
        });
    }

    // static web asset handler
    server_.addHandler(new WebAssetHandler());

    // 404
    server_.onNotFound([](AsyncWebServerRequest* request) {
        request->send( (request->method() == HTTP_OPTIONS) ? 200 : 404 );
    });

    //
    server_.begin();
}

/////////////////////////////////////////////////////////////////////////////
void WebServer::tick() {
    
}
