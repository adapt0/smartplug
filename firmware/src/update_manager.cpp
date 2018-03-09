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
        Serial.println("OTA Start");
        if (onUpdating_) onUpdating_(true);
        inProgress_ = true;
    });
    ArduinoOTA.onError([this](ota_error_t error) {
        Serial.print("\rOTA Error: ");
        Serial.println(error);
        if (onUpdating_) onUpdating_(false);
        inProgress_ = false;
    });
    ArduinoOTA.onEnd([this] {
        Serial.println("\rOTA End  ");
        if (onUpdating_) onUpdating_(false);
        inProgress_ = false;
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("\r%u%%", (progress / (total / 100)));
    });
    ArduinoOTA.begin();
}

void UpdateManager::tick() {
    ArduinoOTA.handle();
}

#endif // UNIT_TEST
