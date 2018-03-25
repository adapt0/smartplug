/////////////////////////////////////////////////////////////////////////////
/** @file
Settings

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////

//- includes
#include "settings.h"

extern "C" unsigned long millis();

/////////////////////////////////////////////////////////////////////////////
Settings::Settings()
: propRelay_{ &propRoot_, "relay" }
, propSys_{ &propRoot_, "sys" }
, propSysSsid_{ &propSys_, "ssid" }
, propTest_{ &propRoot_, "test" }
, propTestInt_{ &propTest_, "int", 42 }
, propPower_{ &propRoot_, "power" }
, propVoltage_{ &propRoot_, "voltage" }
{ }

/////////////////////////////////////////////////////////////////////////////
void Settings::begin() {
    lastMillis_ = millis();
}

/////////////////////////////////////////////////////////////////////////////
void Settings::tick() {
    const auto now = millis();
    if ((now - lastMillis_) < 100) return;
    lastMillis_ = now;

    // process dirty properties
    if (onDirtyProperties_ && propRoot_.dirty()) {
        DynamicJsonBuffer buffer;
        const auto& obj = propRoot_.toJson(buffer, Property::JSON_DIRTY);
        onDirtyProperties_(obj, buffer);
    }
}

/////////////////////////////////////////////////////////////////////////////
/// update measurements
void Settings::updateMeasurements(double watts, double volts) {
    propPower_.setValue(watts);
    propVoltage_.setValue(volts);
}

/////////////////////////////////////////////////////////////////////////////
/// received command
Settings::CommandResult Settings::onCommand(const char* method, const JsonVariant& params, JsonBuffer& buffer) {
    if (0 == strcmp(method, "ping")) {
        return CommandResult(JsonRpcError::NO_ERROR, "pong");

    } else if (0 == strcmp(method, "relay")) {
        const auto newValue = params.as<bool>();
        if (propRelay_.value() != newValue) {
            propRelay_.setValue(newValue);
            if (onRelay_) onRelay_(newValue);
        }
        return CommandResult(JsonRpcError::NO_ERROR, true);

    } else if (0 == strcmp(method, "state")) {
        return CommandResult(JsonRpcError::NO_ERROR, this->toJson(buffer));

    } else if (0 == strcmp(method, "test")) {
        propTestInt_.setValue(propTestInt_.value() + 1);
        return CommandResult(JsonRpcError::NO_ERROR, true);

    } else {
        return CommandResult(JsonRpcError::METHOD_NOT_FOUND, "Method not found");
    }
}
