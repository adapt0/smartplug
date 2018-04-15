/////////////////////////////////////////////////////////////////////////////
/** @file
SSDP

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED__SSDP
#define INCLUDED__SSDP

//- includes
#include <ESP8266SSDP.h>

//- forwards
class AsyncWebServerRequest;

/////////////////////////////////////////////////////////////////////////////
/// evilly expose SSDPClass
class SSDPExt : public SSDPClass {
public:
    SSDPExt() = delete;
    SSDPExt(const SSDPExt&) = delete;
    SSDPExt& operator=(const SSDPExt&) = delete;

    static bool begin();
    static void sendResponse(AsyncWebServerRequest* request);

protected:
    void sendResponse_(AsyncWebServerRequest* request) const;
};

#endif // INCLUDED__SSDP
