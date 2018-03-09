/////////////////////////////////////////////////////////////////////////////
/** @file
ESP8266 Etekcity/Vesync smart plug hacking

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////

//- includes
#include "button.h"
#include "heartbeat.h"
#include "smartplug.h"
#include "update_manager.h"
#include "web_server.h"
#include "wifi_manager.h"
#include <ESP8266mDNS.h>
#include <FS.h>
#include <Settings.h>

//- globals
namespace {
    HeartBeat           heartBeat{SmartPlug::PIN_MOD_LED};
    Settings            settings;
    SmartPlug           smartPlug;
    UpdateManager       updateManager;
    WebServer           webServer{settings};
    WifiManager         wifiManager{SmartPlug::PIN_BLUE_LED};

    bool                otaInProgress = false;

    Button              button{SmartPlug::PIN_SW1, smartPlug, wifiManager, otaInProgress};
}

/////////////////////////////////////////////////////////////////////////////
/// initialization
void setup() {
    Serial.begin(115200);
    while (!Serial);
    Serial.println("SmartPlug v0.01");
    Serial.print("MAC: ");
    Serial.println(WiFi.macAddress());

    //
    settings.begin();

    //
    if (!SPIFFS.begin()) {
        Serial.println("Failed to mount file system");
        return;
    }
// {
//     Dir dir = SPIFFS.openDir("/");
//     while (dir.next()) {    
//         const auto fileName = dir.fileName();
//         const auto fileSize = dir.fileSize();
//         Serial.printf("FS File: %s, size: %d\n", fileName.c_str(), fileSize);
//     }
// }

    wifiManager.begin();
    wifiManager.attachConnected([](const IPAddress& /*ip*/) {
        MDNS.update();
    });

    Serial.print("Hostname: ");
    Serial.println(wifiManager.hostname());

    smartPlug.begin();

    updateManager.attachUpdating([](bool inProgress) {
        if (inProgress) smartPlug.setRelay(false);
    });
    updateManager.begin(wifiManager.hostname());

    button.begin();

    //
    MDNS.addService("http", "tcp", 80);
    MDNS.begin(wifiManager.hostname().c_str());

    webServer.begin();
}

/////////////////////////////////////////////////////////////////////////////
/// main loop
void loop() {
    button.tick();
    heartBeat.tick();
    settings.tick();
    smartPlug.tick();
    updateManager.tick();
    webServer.tick();
    wifiManager.tick();

    yield();
}
