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
#include <ESP8266WiFi.h>
#include <functional>

/////////////////////////////////////////////////////////////////////////////
/// WiFi manager
class WifiManager {
public:
    using NetworkUPtr = Settings::NetworkUPtr;
    using OnConnected = std::function<void (const IPAddress&)>;

    /////////////////////////////////////////////////////////////////////////
    explicit WifiManager(Settings& settings, int pinLed = -1)
    : settings_(settings)
    , pinLed_(pinLed)
    { }

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

    Settings&   settings_;              ///< settings access
    String      hostname_;              ///< our hostname
    String      apPassword_;            ///< our AP's password
    OnConnected onConnected_;           ///< on connected callback
    NetworkUPtr networkToApply_;        ///< new network settings to apply
    int         pinLed_ = -1;           ///< connectivity LED
    bool        staConnected_ = false;  ///< is STA connected?
};

#endif // INCLUDED__WIFIMANAGER
