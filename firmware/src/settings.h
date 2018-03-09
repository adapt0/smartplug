/////////////////////////////////////////////////////////////////////////////
/** @file
Settings

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED__SETTINGS
#define INCLUDED__SETTINGS

//- includes
#include "property.h"

/////////////////////////////////////////////////////////////////////////////
/// persistent settings
class Settings {
public:
    Settings();

    void begin();
    void tick();

    /////////////////////////////////////////////////////////////////////////
    /// output JSON
    JsonObject& toJson(JsonBuffer& buffer) {
        return prop_root_.toJson(buffer);
    }

private:
    PropertyNode prop_root_;
    PropertyNode prop_sys_;
    PropertyNode prop_test_;
    PropertyInt  prop_test_int_;
};

#endif // INCLUDED__SETTINGS
