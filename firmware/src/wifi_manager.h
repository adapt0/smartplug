/////////////////////////////////////////////////////////////////////////////
/** @file
WiFi manager

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED__WIFIMANAGER
#define INCLUDED__WIFIMANAGER

//- includes
#include <ESP8266WiFi.h>
#include <functional>

/////////////////////////////////////////////////////////////////////////////
/// WiFi manager
class WifiManager {
public:
    using OnConnected = std::function<void (const IPAddress&)>;

    /////////////////////////////////////////////////////////////////////////
    explicit WifiManager(int pinLed = -1)
    : pinLed_(pinLed)
    { }

    void begin();
    void tick();

    /// our hostname
    const String& hostname() const { return hostname_; }

    /// connected?
    bool isConnected() const { return staConnected_; }

    /// current WiFi mode
    int mode() const { return mode_; }

    void setModeAP();
    void setModeSTA();

    /// attach connected callback
    void attachConnected(OnConnected onConnected) {
        onConnected_ = std::move(onConnected);
    }

private:
    void updateLed_();

    String      hostname_;              ///< our hostname
    String      apPassword_;            ///< our AP's password
    OnConnected onConnected_;           ///< on connected callback
    int         pinLed_ = -1;           ///< connectivity LED
    int         mode_ = WIFI_OFF;       ///< WiFi mode
    bool        staConnected_ = false;  ///< is STA connected?
};

#endif // INCLUDED__WIFIMANAGER
