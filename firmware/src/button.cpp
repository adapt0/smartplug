/////////////////////////////////////////////////////////////////////////////
/** @file
Button handler

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////
#ifndef UNIT_TEST

//- includes
#include "button.h"
#include "settings.h"
#include "wifi_manager.h"
#include <cassert>

//- statics
Button* Button::instance_ = nullptr;

/////////////////////////////////////////////////////////////////////////////
Button::Button(int pin, Settings& settings, WifiManager& wifiManager, bool& otaInProgress)
: button_(pin, true)
, settings_(settings)
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
        instance_->settings_.setRelay(
            !instance_->settings_.relay()
        );
    });
    // button.attachDoubleClick([] {
    //     printf("button double click");
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
                printf("Button - Switch to AP\r\n");
                instance_->wifiManager_.setModeAP();
            } else {
                printf("Button - Switch to STA\r\n");
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
