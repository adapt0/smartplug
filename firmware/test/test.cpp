/////////////////////////////////////////////////////////////////////////////
/** @file
Unit tests entry point

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest_ext.h"

#include <cstdio>
#include <sys/time.h>

/////////////////////////////////////////////////////////////////////////////
extern "C" unsigned long millis() {
    timeval tv{ };
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

/////////////////////////////////////////////////////////////////////////////
extern "C" char* utoa(unsigned value, char* result, int base) {
    switch (base) {
    case 16:
        sprintf(result, "%x", value);
        return result;
    case 10:
    default:
        sprintf(result, "%u", value);
        return result;
    }
}
extern "C" char* itoa(int value, char* result, int base) {
    switch (base) {
    case 16:
        sprintf(result, "%x", value);
        return result;
    case 10:
    default:
        sprintf(result, "%d", value);
        return result;
    }
}
