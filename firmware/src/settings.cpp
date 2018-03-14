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
: propSys_{ &propRoot_, "sys" }
, propTest_{ &propRoot_, "test" }
, propTestInt_{ &propTest_, "int", 42 }
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
/// received command
Settings::CommandResult Settings::onCommand(const char* method, const JsonObject& params, JsonBuffer& buffer) {
    if (0 == strcmp(method, "ping")) {
        return CommandResult(JsonRpcError::NO_ERROR, "pong");

    } else if (0 == strcmp(method, "state")) {
        return CommandResult(JsonRpcError::NO_ERROR, this->toJson(buffer));

    } else if (0 == strcmp(method, "test")) {
        propTestInt_.setValue(propTestInt_.value() + 1);
        return CommandResult(JsonRpcError::NO_ERROR, true);

    } else {
        return CommandResult(JsonRpcError::METHOD_NOT_FOUND, "Method not found");
    }
}
