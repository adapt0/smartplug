/////////////////////////////////////////////////////////////////////////////
/** @file
Heart beat LED

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED__HEARTBEAT
#define INCLUDED__HEARTBEAT

//- includes
#include <cassert>

/////////////////////////////////////////////////////////////////////////////
/// heart beat LED
class HeartBeat {
public:
    explicit HeartBeat(int pinLed)
    : pinLed_(pinLed)
    { }

    void tick() {
        assert(pinLed_ >= 0);

        static auto lastMillis = millis();
        if ((millis() - lastMillis) >= 1000) {
            lastMillis += 1000;

            static bool heartbeat = false;
            heartbeat = !heartbeat;
            digitalWrite(pinLed_, heartbeat);
        }
    }

private:
    int pinLed_ = -1;
};

#endif // INCLUDED__HEARTBEAT
