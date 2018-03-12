/////////////////////////////////////////////////////////////////////////////
/** @file
ESP8266 Etekcity/Vesync smart plug hacking

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////
#ifndef UNIT_TEST

//- includes
#include "button.h"
#include "console.h"
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
    Console             console{Serial};
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
/// print out version
void printVersion() {
    printf("SmartPlug v0.01\r\n");
}

/////////////////////////////////////////////////////////////////////////////
/// directory listing
void cmdDir(const char*[], int) {
    Dir dir = SPIFFS.openDir("/");
    while (dir.next()) {    
        const auto fileName = dir.fileName();
        const auto fileSize = dir.fileSize();
        printf("FS File: %s, size: %d\r\n", fileName.c_str(), fileSize);
    }
}
/// echo parameters
void cmdEcho(const char* argv[], int argc) {
    for (int i = 0; i < argc; ++i) {
        printf("%d: %s\r\n", i, argv[i]);
    }
}
/// reboot
void cmdReboot(const char*[], int) {
    ESP.reset();
}
/// output our version
void cmdVersion(const char*[], int) {
    printVersion();
    printf("MAC: %s\r\n", WiFi.macAddress().c_str());
    printf("Hostname: %s\r\n", wifiManager.hostname().c_str());
}

/////////////////////////////////////////////////////////////////////////////
/// initialization
void setup() {
    Serial.begin(115200);
    while (!Serial);

    printf("\r\n\r\n");
    printVersion();

    //
    settings.begin();

    //
    if (!SPIFFS.begin()) {
        printf("Failed to mount file system\r\n");
        return;
    }

    wifiManager.begin();
    wifiManager.attachConnected([](const IPAddress& /*ip*/) {
        MDNS.update();
    });

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

    //
    static Console::Command commands[] = {
        { "dir",     &cmdDir },
        { "echo",    &cmdEcho },
        { "help",    &Console::cmdHelp },
        { "reboot",  &cmdReboot },
        { "version", &cmdVersion },
    };
    console.begin(commands, sizeof(commands)/sizeof(commands[0]));
}

/////////////////////////////////////////////////////////////////////////////
/// main loop
void loop() {
    button.tick();
    console.tick();
    heartBeat.tick();
    settings.tick();
    smartPlug.tick();
    updateManager.tick();
    webServer.tick();
    wifiManager.tick();

    yield();
}

#endif // UNIT_TEST
