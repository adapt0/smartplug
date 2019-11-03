/////////////////////////////////////////////////////////////////////////////
/** @file
Button handler

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED__BUTTON
#define INCLUDED__BUTTON

//- includes
#include <OneButton.h>

//- forwards
class Settings;
class WifiManager;

/////////////////////////////////////////////////////////////////////////////
/// button handler
/// https://github.com/mathertel/OneButton
class Button {
public:
    Button(int pin, Settings& settings, WifiManager& wifiManager, bool& otaInProgress);
    ~Button();

    void begin();
    void tick();

private:
    static Button*  instance_;          ///< singleton instance

    OneButton       button_;
    Settings&       settings_;
    WifiManager&    wifiManager_;
    bool&           otaInProgress_;
};

#endif // INCLUDED__BUTTON
