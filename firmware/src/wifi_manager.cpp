/////////////////////////////////////////////////////////////////////////////
/** @file
WiFi manager

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////
#ifndef UNIT_TEST

//- includes
#include "wifi_manager.h"
#include <user_interface.h> // wifi_station_dhcpc_XXX

/////////////////////////////////////////////////////////////////////////////
void WifiManager::begin() {
    // hostname includes chip id
    const String chipId{ESP.getChipId(), HEX};
    hostname_ = "ESP8266-" + chipId;
    apPassword_ = chipId + chipId;

    // begin wifi (restores from SDKs stored settings)
    WiFi.begin();

    //
    updateNetworkSettings_();

    //
    const auto mode = WiFi.getMode();
    if (WIFI_STA == mode && 0 == WiFi.SSID().length()) {
        setModeAP(); // switch over to AP mode if there's no stored SSID available
    }

    // register for new network settings
    settings_.onNetwork([this](Settings::NetworkUPtr&& network) {
        return onNetworkSettings_(std::move(network));
    });
}

/////////////////////////////////////////////////////////////////////////////
void WifiManager::tick() {
    // are there new settings to apply?
    if (networkToApply_) tickApplyNetworkSettings_();

    // periodically check connection status
    if (WIFI_STA == mode()) {
        const bool connected = (WL_CONNECTED == WiFi.status());
        if (connected != staConnected_) {
            staConnected_ = connected;

            if (connected) {
                const auto ip = WiFi.localIP();
                if (onConnected_) onConnected_(ip);
                printf("WiFi Connected (IP: %s)\r\n", ip.toString().c_str());
                updateNetworkSettings_();

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
/// notify Settings of network changes
void WifiManager::updateNetworkSettings_() {

    const bool dhcp = (DHCP_STARTED == wifi_softap_dhcps_status());
    printf("dhcp %d\n", dhcp);

    settings_.updateNetwork(Settings::Network{
        WiFi.hostname(),
        WiFi.SSID(),
        String{}, // password
        WiFi.localIP(),
        WiFi.subnetMask(),
        WiFi.gatewayIP(),
        WiFi.dnsIP(0),
        WiFi.dnsIP(1)
    });
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

/////////////////////////////////////////////////////////////////////////////
/// reconfigure network setting
bool WifiManager::onNetworkSettings_(Settings::NetworkUPtr&& network) {
    // pick up new network settings, delay until main loop
    networkToApply_ = std::move(network);
    return true;
}

/////////////////////////////////////////////////////////////////////////////
/// apply new network setting as part of the main loop
void WifiManager::tickApplyNetworkSettings_() {
    NetworkUPtr network{std::move(networkToApply_)};
    if (!network) return; // sanity

    printf("Applying new network settings...\r\n");

    if (WIFI_STA != WiFi.getMode() || WiFi.SSID() != network->ssid || network->password.length() > 0) {
        const auto res = WiFi.begin(network->ssid.c_str(), network->password.c_str());
        if (!res) printf("Failed to begin a new WiFi connection via WiFi.begin\r\n");
    }

    if (INADDR_NONE == network->ipv4Address) {
        if (DHCP_STOPPED == wifi_station_dhcpc_status()) {
            if (!WiFi.config(0u, 0u, 0u)) printf("Failed to configure DHCP via WiFi.config\r\n");
            if (DHCP_STOPPED == wifi_station_dhcpc_status()) {
                // kick DHCP as WiFi.config doesn't seem to want to do it?
                if (!wifi_station_dhcpc_start()) printf("Failed to start dhcpc\r\n");
            }
        }
    } else {
        const auto res = WiFi.config(network->ipv4Address, network->ipv4Gateway, network->ipv4Subnet);
        if (!res) printf("Failed to set manual IP via WiFi.config\r\n");
    }
}

#endif // UNIT_TEST
