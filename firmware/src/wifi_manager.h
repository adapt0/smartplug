/////////////////////////////////////////////////////////////////////////////
/** @file
WiFi manager

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED__WIFIMANAGER
#define INCLUDED__WIFIMANAGER

//- includes
#include "settings.h"
#include "property.h"
#include <ESP8266WiFi.h>
#include <functional>

/////////////////////////////////////////////////////////////////////////////
/// WiFi manager
class WifiManager {
public:
    using NetworkUPtr = Settings::NetworkUPtr;
    using OnConnected = std::function<void (const IPAddress&)>;

    explicit WifiManager(Settings& settings, int pinLed = -1);

    void begin();
    void tick();

    /// our hostname
    const String& hostname() const { return hostname_; }

    /// connected?
    bool isConnected() const { return staConnected_; }

    IPAddress ipAddress() const;
    int mode() const;

    void setModeOff();
    void setModeAP();
    void setModeSTA();
    void setModeSTA(const char* ssid, const char* pass);

    /// attach connected callback
    void attachConnected(OnConnected onConnected) {
        onConnected_ = std::move(onConnected);
    }

private:
    void disconnect_();
    bool onNetworkSettings_(NetworkUPtr&& network);
    void tickApplyNetworkSettings_();
    void updateLed_();
    void updateNetworkSettings_();

    /////////////////////////////////////////////////////////////////////////
    /// collection of IPv4 address properties
    struct Ipv4Properties {
        explicit Ipv4Properties(PropertyNode* parent, int flags = 0)
        : address{ parent, "ipv4Address", IPAddress{}, flags }
        , subnet{ parent, "ipv4Subnet", IPAddress{}, flags }
        , gateway{ parent, "ipv4Gateway", IPAddress{}, flags }
        , dns1{ parent, "ipv4Dns1", IPAddress{}, flags }
        , dns2{ parent, "ipv4Dns2", IPAddress{}, flags }
        { }

        PropertyIpAddress   address;
        PropertyIpAddress   subnet;
        PropertyIpAddress   gateway;
        PropertyIpAddress   dns1;
        PropertyIpAddress   dns2;
    };

    Settings&       settings_;              ///< settings access

    PropertyString  propSysNetHostname_;
    PropertyString  propSysNetSsid_;
    PropertyBool    propSysNetDhcp_;
    Ipv4Properties  propSysNetIpv4_;
    PropertyNode    propSysNetCur_;
    Ipv4Properties  propSysNetCurIpv4_;

    String          hostname_;              ///< our hostname
    String          apPassword_;            ///< our AP's password
    OnConnected     onConnected_;           ///< on connected callback
    NetworkUPtr     networkToApply_;        ///< new network settings to apply
    const int       pinLed_ = -1;           ///< connectivity LED
    bool            staConnected_ = false;  ///< is STA connected?
};

#endif // INCLUDED__WIFIMANAGER
