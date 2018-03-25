/////////////////////////////////////////////////////////////////////////////
/** @file
WiFi manager

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////
#ifndef UNIT_TEST

//- includes
#include "wifi_manager.h"
#include "settings.h"

/////////////////////////////////////////////////////////////////////////////
void WifiManager::begin() {
    // hostname includes chip id
    const String chipId{ESP.getChipId(), HEX};
    hostname_ = "ESP8266-" + chipId;
    apPassword_ = chipId + chipId;

    // begin wifi (restores from SDKs stored settings)
    WiFi.begin();

    //
    settings_.setSsid(WiFi.SSID());

    //
    const auto mode = WiFi.getMode();
    if (WIFI_STA == mode && 0 == WiFi.SSID().length()) {
        setModeAP(); // switch over to AP mode if there's no stored SSID available
    }
}

/////////////////////////////////////////////////////////////////////////////
void WifiManager::tick() {
    // periodically check connection status
    if (WIFI_STA == mode()) {
        const bool connected = (WL_CONNECTED == WiFi.status());
        if (connected != staConnected_) {
            staConnected_ = connected;

            if (connected) {
                const auto ip = WiFi.localIP();
                if (onConnected_) onConnected_(ip);
                printf("WiFi Connected (IP: %s)\r\n", ip.toString().c_str());
            } else {
                printf("WiFi Disconnected\r\n");
            }
        }
    }

    updateLed_();
}

/////////////////////////////////////////////////////////////////////////////
/// our IP address
IPAddress WifiManager::ipAddress() const {
    switch (mode()) {
    case WIFI_AP:
    case WIFI_AP_STA:
        return WiFi.softAPIP();
    case WIFI_STA:
        return WiFi.localIP();
    case WIFI_OFF:
    default:
        return IPAddress{};
    }
}

/////////////////////////////////////////////////////////////////////////////
/// current WiFi mode
int WifiManager::mode() const {
    return WiFi.getMode();
}

/////////////////////////////////////////////////////////////////////////////
/// update connectivity LED
void WifiManager::updateLed_() {
    if (pinLed_ < 0) return;

    static auto lastMillis = millis();
    if ((millis() - lastMillis) < 500) return;
    lastMillis = millis();

    const auto ledState = [this]{
        // solid blue in AP mode
        if (WIFI_AP == mode()) return HIGH;
        // off when off
        if (WIFI_OFF == mode()) return LOW;

        // leave off when we're happily connected
        if (isConnected()) return LOW;

        // blink blue when attempting to connect
        static bool blink = true;
        blink = !blink;
        return blink ? HIGH : LOW;
    }();
    digitalWrite(pinLed_, ledState);
}

/////////////////////////////////////////////////////////////////////////////
/// disconnect wifi
void WifiManager::disconnect_() {
    if (staConnected_) printf("WiFi Disconnected\r\n");
    staConnected_ = false;

    WiFi.disconnect();
    WiFi.waitForConnectResult();
}

/////////////////////////////////////////////////////////////////////////////
/// turn off wifi
void WifiManager::setModeOff() {
    disconnect_();

    WiFi.mode(WIFI_OFF);
    WiFi.waitForConnectResult();
}

/////////////////////////////////////////////////////////////////////////////
/// set AP mode
void WifiManager::setModeAP()  {
    disconnect_();

    WiFi.mode(WIFI_AP);
    WiFi.waitForConnectResult();

    if (WiFi.softAP(hostname_.c_str(), apPassword_.c_str())) {
        const auto ip = WiFi.softAPIP();
        printf("AP '%s' (IP: %s)\r\n", hostname_.c_str(), ip.toString().c_str());
    } else {
        printf("Failed to set AP mode\r\n");
    }
}

/////////////////////////////////////////////////////////////////////////////
/// set STA
void WifiManager::setModeSTA() {
    setModeSTA(WiFi.SSID().c_str(), WiFi.psk().c_str());
}
/// set STA
void WifiManager::setModeSTA(const char* ssid, const char* pass) {
    disconnect_();

    WiFi.mode(WIFI_STA);
    WiFi.waitForConnectResult();

    WiFi.begin(ssid, pass);
}

#endif // UNIT_TEST
