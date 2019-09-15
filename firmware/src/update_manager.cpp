/////////////////////////////////////////////////////////////////////////////
/** @file
Firmware update manager

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////
#ifndef UNIT_TEST

//- includes
#include "update_manager.h"
#include <ArduinoOTA.h>

/////////////////////////////////////////////////////////////////////////////
void UpdateManager::begin(const String& hostname) {
    // OTA server
    ArduinoOTA.setHostname(hostname.c_str());
    ArduinoOTA.onStart([this] {
        Serial.print("OTA Start\r\n");
        if (onUpdating_) onUpdating_(true);
        lastPercent_ = -1;
        inProgress_ = true;
    });
    ArduinoOTA.onError([this](ota_error_t error) {
        Serial.printf("\rOTA Error: %d\r\n", error);
        if (onUpdating_) onUpdating_(false);
        inProgress_ = false;
    });
    ArduinoOTA.onEnd([this] {
        Serial.print("\rOTA End  \r\n");
        if (onUpdating_) onUpdating_(false);
        inProgress_ = false;
    });
    ArduinoOTA.onProgress([this](int progress, int total) {
        const int percent = progress / (total / 100);
        if (lastPercent_ == percent) return;
        lastPercent_ = percent;
        Serial.printf("\r%u%%", percent);
    });
    ArduinoOTA.begin(false); // without mDNS
}

void UpdateManager::tick() {
    ArduinoOTA.handle();
}

#endif // UNIT_TEST
