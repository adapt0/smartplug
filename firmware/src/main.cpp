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
#include "settings.h"
#include "smartplug.h"
#include "update_manager.h"
#include "version.h"
#include "web_server.h"
#include "wifi_manager.h"
#include <FS.h>
#include <settings.h>
#include <user_interface.h> // wifi_station_dhcpc_XXX

//- globals
namespace {
    Console             console{Serial};
    HeartBeat           heartBeat{SmartPlug::PIN_MOD_LED};
    Settings            settings;
    SmartPlug           smartPlug{settings};
    UpdateManager       updateManager;
    WebServer           webServer{settings};
    WifiManager         wifiManager{settings, SmartPlug::PIN_BLUE_LED};

    bool                otaInProgress = false;

    Button              button{SmartPlug::PIN_SW1, smartPlug, wifiManager, otaInProgress};
}

/////////////////////////////////////////////////////////////////////////////
/// print out version
void printVersion() {
    printf("SmartPlug %s\r\n", version::STRING_FULL);
}

/////////////////////////////////////////////////////////////////////////////
/// cat file contents
void cmdCat(const char* argv[], int argc) {
    if (argc <= 0) {
        printf("cat (filename)\r\n");
        return;
    }

    File f = SPIFFS.open(argv[0], "r");
    if (f) {
        char buf[32];
        for (size_t ofs = 0; ofs < f.size(); ) {
            const auto tot = f.readBytes(buf, sizeof(buf));
            if (tot <= 0) break;

            Serial.write(reinterpret_cast<const uint8_t*>(buf), tot);
            ofs += tot;
        }
        Serial.println();
    } else {
        printf("Failed to open file\r\n");
    }
}
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
/// free memory
void cmdFree(const char*[], int) {
    
}
/// reboot
void cmdReboot(const char*[], int) {
    settings.setNeedReboot();
}
/// dump state
void cmdState(const char*[], int) {
    DynamicJsonDocument doc{Settings::JSON_STATE_SIZE};
    settings.toJson(doc);
    serializeJson(doc, Serial);
    Serial.println();
}
/// output our version
void cmdVersion(const char*[], int) {
    printVersion();
    printf("MAC: %s\r\n", WiFi.macAddress().c_str());
    printf("Hostname: %s\r\n", wifiManager.hostname().c_str());
}
/// wifi control
void cmdWifi(const char* argv[], int argc) {
    if (argc <= 0) {
        switch (wifiManager.mode()) {
        case WIFI_OFF:
            printf("WIFI: OFF\r\n");
            break;
        case WIFI_STA:
            printf("WIFI: STA\r\n");
            printf("Connected: %d\r\n", wifiManager.isConnected() ? 1 : 0);
            printf("DHCP: %d\r\n", wifi_softap_dhcps_status());
            printf("IP: %s\r\n", wifiManager.ipAddress().toString().c_str());
            break;
        case WIFI_AP:
            printf("WIFI: AP\r\n");
            printf("SSID: %s\r\n", wifiManager.hostname().c_str());
            printf("IP: %s\r\n", wifiManager.ipAddress().toString().c_str());
            break;
        case WIFI_AP_STA:
            printf("WIFI: AP_STA\r\n");
            printf("IP: %s\r\n", wifiManager.ipAddress().toString().c_str());
            break;
        default:
            printf("WIFI: Unknown\r\n");
            break;
        }
        return;
    }

    // turn off wifi
    if (1 == argc && 0 == strcasecmp(argv[0], "off")) {
        wifiManager.setModeOff();
        return;
    }
    // AP mode
    if (1 == argc && 0 == strcasecmp(argv[0], "ap")) {
        wifiManager.setModeAP();
        return;
    }
    // STA mode
    if (3 == argc && 0 == strcasecmp(argv[0], "sta")) {
        wifiManager.setModeSTA(argv[1], argv[2]);
        return;
    }

    printf("wifi OFF\r\n");
    printf("wifi AP\r\n");
    printf("wifi STA (SSID) (PASS)\r\n");
}

/////////////////////////////////////////////////////////////////////////////
/// load settings
void loadSettings() {
    if (!SPIFFS.begin()) {
        printf("Failed to mount file system\r\n");
        return;
    }

    //
    File configFile = SPIFFS.open("/config.json", "r");
    if (configFile) {
        printf("Loading config...\r\n");
        settings.loadFrom(configFile);
    }

    //
    settings.onPersistProperties([](const JsonDocument& docProps) {
        File configFile = SPIFFS.open("/config.json", "w");
        if (configFile) serializeJson(docProps, configFile);
    });
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
    loadSettings();

    //
    printf("Initialize WiFi...\r\n");
    wifiManager.begin();

    smartPlug.begin();

    printf("Starting OTA...\r\n");
    updateManager.attachUpdating([](bool inProgress) {
        if (inProgress) smartPlug.setRelay(false);
    });
    updateManager.begin(wifiManager.hostname());

    button.begin();

    //
    printf("Starting web server...\r\n");
    webServer.begin(wifiManager);

    //
    static Console::Command commands[] = {
        { "cat",     &cmdCat },
        { "dir",     &cmdDir },
        { "echo",    &cmdEcho },
        { "free",    &cmdFree },
        { "help",    &Console::cmdHelp },
        { "reboot",  &cmdReboot },
        { "state",   &cmdState },
        { "wifi",    &cmdWifi },
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

    if (settings.needReboot()) ESP.reset();

    yield();
}

#endif // UNIT_TEST
