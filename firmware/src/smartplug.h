/////////////////////////////////////////////////////////////////////////////
/** @file
ESP8266 Etekcity/Vesync smart plug hacking

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED__SMARTPLUG
#define INCLUDED__SMARTPLUG

//- includes
#include <cassert>

/////////////////////////////////////////////////////////////////////////////
/// smart plug
class SmartPlug {
public:
    enum Pins {
        PIN_MOD_LED   = 2,  ///< ESP8266 module LED
        PIN_RELAY     = 4,  ///< relay (also lights LED yellow)
        PIN_BLUE_LED  = 5,  ///< lights blue LED
        PIN_CF1       = 12, ///< HLW8012 CF1 pin (voltage/current 50% duty cycle)
        PIN_CF        = 13, ///< HLW8012 CF pin  (pulse for active power)
        PIN_SW1       = 14, ///< button
    };

    SmartPlug();
    ~SmartPlug();

    void begin();
    void tick();

    bool relay() const { return relay_; }
    void setRelay(bool state);

private:
    static void onRisingInterrupt_();
    static void onFallingInterrupt_();

    static SmartPlug* instance_;

    volatile double measPower_ = 0;     ///< measured power (W)
    volatile double measVoltage_ = 0;   ///< measured mains voltage (V)

    bool    relay_ = false;             ///< current relay state
};

#endif // INCLUDED__SMARTPLUG
