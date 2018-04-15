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
#include "ssdp.h"
#include <ESP8266mDNS.h> // mDNS
#include <user_interface.h> // wifi_station_dhcpc_XXX

/////////////////////////////////////////////////////////////////////////////
WifiManager::WifiManager(Settings& settings, int pinLed)
: settings_(settings)
, propSysNetHostname_{ &settings.propSysNet(), "hostname", String{}, Property::PERSIST }
, propSysNetSsid_{ &settings.propSysNet(), "ssid" }
, propSysNetDhcp_{ &settings.propSysNet(), "dhcp", true, Property::PERSIST }
, propSysNetIpv4_{ &settings.propSysNet(), Property::PERSIST }
, propSysNetCur_{ &settings.propSysNet(), "cur" }
, propSysNetCurIpv4_{ &propSysNetCur_ }
, pinLed_(pinLed)
{ }

/////////////////////////////////////////////////////////////////////////////
void WifiManager::begin() {
    // AP hostname includes chip id
    const String chipId{ESP.getChipId(), HEX};
    apHostname_ = "ESP8266-" + chipId;
    apPassword_ = chipId + chipId;

    // restore/update persisted host name
    if (propSysNetHostname_->length()) {
        WiFi.hostname(propSysNetHostname_.value());
    } else {
        propSysNetHostname_.set(WiFi.hostname());
    }

    // begin wifi (restores from SDKs stored settings)
    WiFi.begin();
    propSysNetSsid_.set(WiFi.SSID());

    // configure static
    if (false == propSysNetDhcp_.value() && INADDR_NONE != propSysNetIpv4_.address.value()) {
        WiFi.config(
            propSysNetIpv4_.address.value(),
            propSysNetIpv4_.gateway.value(),
            propSysNetIpv4_.subnet.value(),
            propSysNetIpv4_.dns1.value(),
            propSysNetIpv4_.dns2.value()
        );
    }

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

    // start mDNS
    MDNS.addService("http", "tcp", 80);
    if (!MDNS.begin(WiFi.hostname().c_str())) {
        printf("mDNS failed to start\r\n");
    }

    // start SSDP
    if (!SSDPExt::begin()) {
        printf("SSDP failed to start\r\n");
    }
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
                printf("WiFi Connected (IP: %s)\r\n", ip.toString().c_str());
                updateNetworkSettings_();

                // restart SSDP
                if (!SSDP.begin()) {
                    printf("SSDP failed to start\r\n");
                }

                // update mDNS
                MDNS.update();

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
    // const bool dhcp = (DHCP_STARTED == wifi_softap_dhcps_status());
    // printf("dhcp %d\r\n", dhcp);

    // populate our properties with the updated settings
    // propSysNetDhcp_.set(dhcp || INADDR_NONE == network->ipv4Address);
    // if (!dhcp) propSysNetIpv4_.set(*network);

    // propSysNetHostname_.set(std::move(network.hostname));
    // propSysNetSsid_.set(std::move(network.ssid));
    // propSysNetCurIpv4_.set(network);

    // update current IP
    propSysNetCurIpv4_.address.set(WiFi.localIP());
    propSysNetCurIpv4_.subnet.set(WiFi.subnetMask());
    propSysNetCurIpv4_.gateway.set(WiFi.gatewayIP());
    propSysNetCurIpv4_.dns1.set(WiFi.dnsIP(0));
    propSysNetCurIpv4_.dns2.set(WiFi.dnsIP(1));
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

    if (WiFi.softAP(apHostname_.c_str(), apPassword_.c_str())) {
        const auto ip = WiFi.softAPIP();
        printf("AP '%s' (IP: %s)\r\n", apHostname_.c_str(), ip.toString().c_str());
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
        propSysNetSsid_.set(network->ssid);
        const auto res = WiFi.begin(network->ssid.c_str(), network->password.c_str());
        if (!res) printf("Failed to begin a new WiFi connection via WiFi.begin\r\n");
    }

    // update hostname
    {
        //TODO: sanitize hostname
        propSysNetHostname_.set(network->hostname.length() ? network->hostname : apHostname_);
        if (propSysNetHostname_.value() != WiFi.hostname()) {
            WiFi.hostname(propSysNetHostname_.value());
            MDNS.setInstanceName(WiFi.hostname());
            MDNS.begin(WiFi.hostname().c_str());
            MDNS.notifyAPChange();
        }
    }

    // IP settings
    if (INADDR_NONE == network->ipv4Address) {
        propSysNetDhcp_.set(true);
        if (DHCP_STOPPED == wifi_station_dhcpc_status()) {
            if (!WiFi.config(0u, 0u, 0u)) printf("Failed to configure DHCP via WiFi.config\r\n");
            if (DHCP_STOPPED == wifi_station_dhcpc_status()) {
                // kick DHCP as WiFi.config doesn't seem to want to do it?
                if (!wifi_station_dhcpc_start()) printf("Failed to start dhcpc\r\n");
            }
        }
    } else {
        propSysNetDhcp_.set(false);
        propSysNetIpv4_.address.set(network->ipv4Address);
        propSysNetIpv4_.gateway.set(network->ipv4Gateway);
        propSysNetIpv4_.subnet.set(network->ipv4Subnet);
        propSysNetIpv4_.dns1.set(network->ipv4Dns1);
        propSysNetIpv4_.dns2.set(network->ipv4Dns2);

        const auto res = WiFi.config(
            propSysNetIpv4_.address.value(),
            propSysNetIpv4_.gateway.value(),
            propSysNetIpv4_.subnet.value(),
            propSysNetIpv4_.dns1.value(),
            propSysNetIpv4_.dns2.value()
        );
        if (!res) printf("Failed to set manual IP via WiFi.config\r\n");
    }
}

#endif // UNIT_TEST
