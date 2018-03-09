/////////////////////////////////////////////////////////////////////////////
/** @file
Button handler

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////
#ifndef UNIT_TEST

//- includes
#include "button.h"
#include "smartplug.h"
#include "wifi_manager.h"
#include <cassert>

//- statics
Button* Button::instance_ = nullptr;

/////////////////////////////////////////////////////////////////////////////
Button::Button(int pin, SmartPlug& smartPlug, WifiManager& wifiManager, bool& otaInProgress)
: button_(pin, true)
, smartPlug_(smartPlug)
, wifiManager_(wifiManager)
, otaInProgress_(otaInProgress)
{
    assert(!instance_);
    instance_ = this;
}

/// destructor
Button::~Button() {
    instance_ = nullptr;
}

/////////////////////////////////////////////////////////////////////////////
void Button::begin() {
    // one button callbacks doesn't allow for lambda captures :(
    button_.attachClick([] {
        if (instance_->otaInProgress_) return;
        instance_->smartPlug_.setRelay(
            !instance_->smartPlug_.relay()
        );
    });
    // button.attachDoubleClick([] {
    //     Serial.println("button double click");
    // });
    static bool longPressed = false;
    static int longMillis = -1;
    button_.attachLongPressStart([] {
        longMillis = millis();
        longPressed = false;
    });
    button_.attachDuringLongPress([] {
        if (!longPressed && (millis() - longMillis) > (5000 - 1000)) {
            longPressed = true;

            // switch to/from AP mode
            if (WIFI_AP != instance_->wifiManager_.mode()) {
                instance_->wifiManager_.setModeAP();
            } else {
                instance_->wifiManager_.setModeSTA();
            }
        }
    });
}

/////////////////////////////////////////////////////////////////////////////
void Button::tick() {
    button_.tick();
}

#endif // UNIT_TEST
