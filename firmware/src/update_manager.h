/////////////////////////////////////////////////////////////////////////////
/** @file
Firmware update manager

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED__UPDATEMANAGER
#define INCLUDED__UPDATEMANAGER

//- includes
#include <functional>

//- forwards
class String;

/////////////////////////////////////////////////////////////////////////////
/// firmware update manager
class UpdateManager {
public:
    using OnUpdating = std::function<void (bool)>;

    void begin(const String& hostname);
    void tick();

    /// attach connected callback
    void attachUpdating(OnUpdating onUpdating) {
        onUpdating_ = std::move(onUpdating);
    }

private:
    OnUpdating  onUpdating_;            ///< on updating callback
    int         lastPercent_ = -1;      ///< last progress %
    bool        inProgress_ = false;    ///< OTA in progress?
};

#endif // INCLUDED__UPDATEMANAGER
