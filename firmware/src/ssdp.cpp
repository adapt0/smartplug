/////////////////////////////////////////////////////////////////////////////
/** @file
SSDP

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////
#ifndef UNIT_TEST

//- includes
#include "ssdp.h"
#include <ESPAsyncWebServer.h>

/////////////////////////////////////////////////////////////////////////////
/// send response
bool SSDPExt::begin() {
    SSDP.setSchemaURL("description.xml");
    SSDP.setHTTPPort(80);
    SSDP.setDeviceType("upnp:rootdevice");
    SSDP.setName("SmartPlug");
    SSDP.setSerialNumber("");
    SSDP.setURL("/");
    SSDP.setModelName("SmartPlug");
    SSDP.setModelNumber("SmartPlug");
    SSDP.setModelURL("http://www.github.com/adapt0/smartplug");
    SSDP.setManufacturer("Manufacturer");
    SSDP.setManufacturerURL("http://www.github.com/adapt0/smartplug");

    return SSDP.begin();
}

/////////////////////////////////////////////////////////////////////////////
/// send response
void SSDPExt::sendResponse(AsyncWebServerRequest* request) {
    auto* evil = reinterpret_cast<const SSDPExt*>(&SSDP);
    evil->sendResponse_(request);
}
/// send response
void SSDPExt::sendResponse_(AsyncWebServerRequest* request) const {
    // reimplement SSDPClass::schema for use with AsyncWebServerRequest
    static const char SSDP_TEMPLATE[] =
        "<?xml version=\"1.0\"?>"
        "<root xmlns=\"urn:schemas-upnp-org:device-1-0\">"
            "<specVersion>"
                "<major>1</major>"
                "<minor>0</minor>"
            "</specVersion>"
            "<URLBase>http://%u.%u.%u.%u:%u/</URLBase>" // WiFi.localIP(), _port
            "<device>"
                "<deviceType>%s</deviceType>"
                "<friendlyName>%s</friendlyName>"
                "<presentationURL>%s</presentationURL>"
                "<serialNumber>%s</serialNumber>"
                "<modelName>%s</modelName>"
                "<modelNumber>%s</modelNumber>"
                "<modelURL>%s</modelURL>"
                "<manufacturer>%s</manufacturer>"
                "<manufacturerURL>%s</manufacturerURL>"
                "<UDN>uuid:%s</UDN>"
            "</device>"
        "</root>"
    ;

    IPAddress ip = WiFi.localIP();
    char buffer[sizeof(SSDP_TEMPLATE) + sizeof(SSDPClass)];
    sprintf(buffer, SSDP_TEMPLATE,
        ip[0], ip[1], ip[2], ip[3], _port,
        _deviceType,
        _friendlyName,
        _presentationURL,
        _serialNumber,
        _modelName,
        _modelNumber,
        _modelURL,
        _manufacturer,
        _manufacturerURL,
        _uuid
    );

    request->send(200, "text/xml", buffer);    
}

#endif // UNIT_TEST
