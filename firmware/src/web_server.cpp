/////////////////////////////////////////////////////////////////////////////
/** @file
Web Server

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////
#ifndef UNIT_TEST

//- includes
#include "web_server.h"
#include "settings.h"
#include "ssdp.h"
#include "web_server_asset_handler.h"
#include "wifi_manager.h"
#include <ArduinoJson.h>

/////////////////////////////////////////////////////////////////////////////
/// log web requests
class WebRequestLogger : public AsyncWebHandler {
public:
    /////////////////////////////////////////////////////////////////////////
    /// constructor
    WebRequestLogger() { }
    /// destructor
    ~WebRequestLogger() override = default;

    /////////////////////////////////////////////////////////////////////////
    /// can we handle request?
    bool canHandle(AsyncWebServerRequest* request) override final {
        printf("%s %s\r\n", request->methodToString(), request->url().c_str());
        return false;
    }
};


/////////////////////////////////////////////////////////////////////////////
/// constructor
WebServer::WebServer(Settings& settings)
: serverWebSocket_("/api/v1")
, settings_(settings)
{ }

/////////////////////////////////////////////////////////////////////////////
/// begin web server
void WebServer::begin(WifiManager& wifi) {
    // request logger
    server_.addHandler(new WebRequestLogger());

    // API requests
    {
        server_.on("/api/v1/ping", HTTP_GET, [](AsyncWebServerRequest* request) {
            request->send(200, "text/plain", "pong");
        });
        server_.on("/api/v1/state", HTTP_GET, [this](AsyncWebServerRequest* request) {
            auto* response = request->beginResponseStream("application/json");
            if (response) {
                DynamicJsonDocument doc{Settings::JSON_STATE_SIZE};
                settings_.toJson(doc);
                serializeJson(doc, *response);
                request->send(response);
            }
        });
        server_.on("/api/v1/update", HTTP_POST, [this](AsyncWebServerRequest* request) {
            // HTTP update based on https://gist.github.com/JMishou/60cb762047b735685e8a09cd2eb42a60

            const bool success = (update_request_ == request && !Update.hasError());

            // request handler is triggered after the upload has finished
            auto* response = request->beginResponse(200, "text/plain", (success) ? "OK" : "FAIL");
            response->addHeader("Connection", "close");
            request->send(response);

            if (success) {
                serverWebSocket_.enable(false);
                serverWebSocket_.closeAll();

                printf("Rebooting...\r\n");
                settings_.setNeedReboot(); // Tell the main loop to restart the ESP
            }

            // finished with interlock
            if (update_request_ == request) update_request_ = nullptr;

        },[this](AsyncWebServerRequest* request, const String& filename, size_t index, uint8_t* data, size_t len, bool final) {
            //Upload handler chunks in data

            if (0 == index) { // if index == 0 then this is the first frame of data
                printf("Update started '%s'\r\n", filename.c_str());

                // block out concurrent update requests
                if (update_request_) {
                    printf("Update request is already in progress!\r\n");
                    return;
                }
                update_request_ = request;
                last_update_percent_ = -1;

                // calculate sketch space required for the update
                uint32_t maxSketchSpace = (ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000;
                if (!Update.begin(maxSketchSpace)) { //start with max available size
                    Update.printError(Serial);
                    return;
                }
                Update.runAsync(true); // tell the updaterClass to run in async mode
            }

            // ignore body if request doesn't match initiator
            if (update_request_ != request) return;

            // show progress if we have a content length
            {
                const auto contentLength = request->contentLength();
                if (contentLength > 0) {
                    const int percentage = index * 100 / contentLength;
                    if (percentage != last_update_percent_) {
                        last_update_percent_ = percentage;
                        printf("Upload: %d%%        \r", percentage);
                    }
                }
            }

            // Write chunked data to the free sketch space
            if (Update.write(data, len) != len) {
                Update.printError(Serial);
                update_request_ = nullptr;
                return;
            }

            if (final) { // if the final flag is set then this is the last frame of data
                if (Update.end(true)) { //true to set the size to the current progress
                    printf("Update success: %u\r\n", index+len);
                } else {
                    Update.printError(Serial);
                    update_request_ = nullptr;
                    return;
                }
                Serial.setDebugOutput(false);
            }
        });

        // async WebSocket Event
        serverWebSocket_.onEvent([this](AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
            this->onWebSocketEvent_(server, client, type, arg, data, len);
        });
        server_.addHandler(&serverWebSocket_);

        // no match
        server_.on("/api", [](AsyncWebServerRequest* request) {
            request->send(404);
        });
    }

    // SSDP device profile
    server_.on("/description.xml", HTTP_GET, [](AsyncWebServerRequest* request) {
        SSDPExt::sendResponse(request);
    });

    // static web asset handler
    server_.addHandler(new WebAssetHandler());

    // 404
    server_.onNotFound([](AsyncWebServerRequest* request) {
        request->send( (request->method() == HTTP_OPTIONS) ? 200 : 404 );
    });

    // dirty property notifications
    settings_.onDirtyProperties([this](const JsonDocument& docProps) {
        DynamicJsonDocument doc{Settings::JSON_STATE_SIZE};
        auto json = doc.to<JsonObject>();
        json["jsonrpc"] = "2.0";
        json["method"] = "update";
        json["params"] = docProps;
        auto* textBuffer = serverWebSocket_.makeBuffer(measureJson(json));
        if (textBuffer) {
            serializeJson(json, reinterpret_cast<char*>(textBuffer->get()), textBuffer->length() + 1);
            serverWebSocket_.textAll(textBuffer);
        }
    });

    // restart server on network changes
    wifi.onNetwork([this](bool connected) {
        if (!connected) {
            serverWebSocket_.closeAll();
            serverWebSocket_.cleanupClients(0);
        }
    });

    //
    server_.begin();
}

/////////////////////////////////////////////////////////////////////////////
void WebServer::tick() {
    // periodically clean up old clients
    serverWebSocket_.cleanupClients();
}

/////////////////////////////////////////////////////////////////////////////
/// web socket event
void WebServer::onWebSocketEvent_(AsyncWebSocket* server, AsyncWebSocketClient* client, AwsEventType type, void* arg, uint8_t* data, size_t len) {
    if (type == WS_EVT_CONNECT) {
        printf("ws[%s][%u] connect\r\n", server->url(), client->id());
        // client->printf("Hello Client %u :)", client->id());
        // client->ping();
    } else if (type == WS_EVT_DISCONNECT) {
        printf("ws[%s][%u] disconnect\r\n", server->url(), client->id());
    } else if (type == WS_EVT_ERROR) {
        printf("ws[%s][%u] error(%u): %s\r\n", server->url(), client->id(), *((uint16_t*)arg), (char*)data);
    } else if (type == WS_EVT_PONG) {
        printf("ws[%s][%u] pong[%u]: %s\r\n", server->url(), client->id(), len, (len)?(char*)data:"");
    } else if (type == WS_EVT_DATA) {
        // printf("ws[%s][%u] data\r\n", server->url(), client->id());

        auto* info = (AwsFrameInfo*)arg;
        if (info->opcode != WS_TEXT) return; // only interested in text frames

        data[len] = 0; // null terminate

        if (info->final && 0 == info->index && info->len == len) {
            //the whole message is in a single frame and we got all of it's data
            // printf("ws[%s][%u] %s-message[%llu]: ", server->url(), client->id(), (info->opcode == WS_TEXT) ? "text" : "binary", info->len);
            onJsonRpc_(client, (char*)data, len);

        } else {
            //message is comprised of multiple frames or the frame is split into multiple packets
            if (info->index == 0) {
                if (info->num == 0) {
                    printf("ws[%s][%u] %s-message start\r\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
                }
                printf("ws[%s][%u] frame[%u] start[%llu]\r\n", server->url(), client->id(), info->num, info->len);
            }
            printf("ws[%s][%u] frame[%u] %s[%llu - %llu]: ", server->url(), client->id(), info->num, (info->message_opcode == WS_TEXT)?"text":"binary", info->index, info->index + len);
            if ((info->index + len) == info->len) {
                printf("ws[%s][%u] frame[%u] end[%llu]\r\n", server->url(), client->id(), info->num, info->len);
                if (info->final) {
                    printf("ws[%s][%u] %s-message end\r\n", server->url(), client->id(), (info->message_opcode == WS_TEXT)?"text":"binary");
                    if (info->message_opcode == WS_TEXT) {
                        client->text("I got your text message");
                    } else {
                        client->binary("I got your binary message");
                    }
                }
            }
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
/// on JSON-RPC data
void WebServer::onJsonRpc_(AsyncWebSocketClient* client, char* data, size_t len) {
    // parse JSON
    DynamicJsonDocument request{Settings::JSON_REQUEST_SIZE};
    if (DeserializationError::Ok != deserializeJson(request, data, len)) return; // :(

    // serializeJson(request, Serial);

    // A String specifying the version of the JSON-RPC protocol. MUST be exactly "2.0".
    {
        const char* jsonrpc = request["jsonrpc"];
        if (!jsonrpc || 0 != strcmp(jsonrpc, "2.0")) return;
    }

    // A String containing the name of the method to be invoked
    const char* method = request["method"];
    if (!method) return;
    // A Structured value that holds the parameter values to be used during the invocation of the method. This member MAY be omitted
    const JsonVariant& params = request["params"];
    // An identifier established by the Client that MUST contain a String, Number, or NULL value if included
    const auto id = request["id"];

    // process request
    {
        DynamicJsonDocument resultDoc{Settings::JSON_STATE_SIZE};
        const auto result = settings_.call(method, params, resultDoc);

        // fill in response
        DynamicJsonDocument responseDoc{Settings::JSON_STATE_SIZE};
        auto response = responseDoc.to<JsonObject>();
        response["jsonrpc"] = "2.0";
        response["id"] = id;

        if (JsonRpcError::NO_ERROR == result) {
            response["result"] = resultDoc;
        } else {
            auto error = response.createNestedObject("error");
            error["code"] = static_cast<int>(result);
            error["message"] = resultDoc;
        }

        // serializeJson(response, Serial);

        // send response
        auto* buffer = serverWebSocket_.makeBuffer(measureJson(response));
        if (buffer) {
            serializeJson(response, reinterpret_cast<char*>(buffer->get()), buffer->length() + 1); // room for null terminator (buffer included this for us)
            client->text(buffer);
        }
    }
}

#endif // UNIT_TEST
