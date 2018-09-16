/////////////////////////////////////////////////////////////////////////////
/** @file
Web Server static asset handler

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED__WEB_SERVER_ASSET_HANDLER
#define INCLUDED__WEB_SERVER_ASSET_HANDLER

//- includes
#include "web_assets.h"
#include <algorithm>
#include <ESPAsyncWebServer.h>

/////////////////////////////////////////////////////////////////////////////
/// serves precompiled WebAssets
class WebAssetHandler: public AsyncWebHandler {
public:
    /////////////////////////////////////////////////////////////////////////
    /// constructor
    explicit WebAssetHandler(const char* uri = "") : uri_(uri) { }
    /// destructor
    ~WebAssetHandler() override = default;

    /////////////////////////////////////////////////////////////////////////
    /// can we handle request?
    bool canHandle(AsyncWebServerRequest* request) override final {
        if (   HTTP_GET != request->method()
            || !request->url().startsWith(uri_) 
            || !request->isExpectedRequestedConnType(RCT_DEFAULT, RCT_HTTP)
        ) {
            return false;
        }

        return !!getAssetByPath_(request->url());
    }

    /////////////////////////////////////////////////////////////////////////
    /// handle actual request
    void handleRequest(AsyncWebServerRequest* request) override final {
        const auto* asset = getAssetByPath_(request->url());
        if (asset) {
            auto* response = request->beginResponse_P(200, asset->mimeType, asset->data, asset->length);
            response->addHeader("Content-Encoding", "gzip");
            request->send(response);
        } else {
            request->send(404);
        }
    }

private:
    /////////////////////////////////////////////////////////////////////////
    /// search for matching asset
    const WebAsset* getAssetByPath_(const String& path) {
        // append a default file if only a directory has been specified
        if (path.length() > 0 && '/' == path[path.length() - 1]) {
            return getAssetByPath_(path + "index.html");
        }

        // assumes webAssets have been pre-sorted
        const auto fnd = std::equal_range(
            webAssets, webAssets + webAssetsCount,
            path,
            WebAssetPathCompare()
        );
        return (fnd.first != fnd.second) ? &*fnd.first : nullptr;
    }

    const String uri_;  ///< parent URI
};

#endif // INCLUDED__WEB_SERVER_ASSET_HANDLER
