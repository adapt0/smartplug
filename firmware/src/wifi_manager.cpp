/////////////////////////////////////////////////////////////////////////////
/** @file
WiFi manager

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////
#ifndef UNIT_TEST

//- includes
#include "wifi_manager.h"

/////////////////////////////////////////////////////////////////////////////
void WifiManager::begin() {
    // hostname includes chip id
    const String chipId(ESP.getChipId(), HEX);
    hostname_ = "ESP8266-" + chipId;
    apPassword_ = chipId + chipId;

    // begin wifi
    WiFi.begin();

    // update our cached internal state based on esp8266 restored persisted state
    if (WIFI_STA == WiFi.getMode()) {
        mode_ = WIFI_STA;
    } else {
        setModeAP();
    }
}

/////////////////////////////////////////////////////////////////////////////
void WifiManager::tick() {
    if (WIFI_STA == mode_) {
        const bool connected = (WL_CONNECTED == WiFi.status());
        if (connected != staConnected_) {
            staConnected_ = connected;

            if (connected) {
                const auto ip = WiFi.localIP();
                if (onConnected_) onConnected_(ip);
                Serial.print("WiFi Connected (IP: ");
                Serial.print(ip);
                Serial.println(')');
            } else {
                Serial.println("WiFi Disconnected");
            }
        }
    }

    updateLed_();
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
/// set AP mode
void WifiManager::setModeAP()  {
    mode_ = WIFI_AP;
    staConnected_ = false;

    Serial.print("AP '");
    Serial.print(hostname_);
    Serial.print("' ");

    WiFi.disconnect();
    WiFi.waitForConnectResult();

    WiFi.mode(WIFI_AP);
    if (WiFi.softAP(hostname_.c_str(), apPassword_.c_str())) {
        const auto ip = WiFi.softAPIP();
        Serial.print(" (IP: ");
        Serial.print(ip);
        Serial.print(')');
    }
    Serial.println();
}

/////////////////////////////////////////////////////////////////////////////
/// set STA
void WifiManager::setModeSTA() {
    mode_ = WIFI_STA;
    staConnected_ = false;

    WiFi.begin(ssid.c_str(), password.c_str());
}

#endif // UNIT_TEST
