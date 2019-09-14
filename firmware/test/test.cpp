/////////////////////////////////////////////////////////////////////////////
/** @file
Unit tests entry point

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest_ext.h"

#include <cstdio>
#include <ostream>
#include <sys/time.h>

extern "C" {

const ip_addr_t ip_addr_any = IPADDR4_INIT(IPADDR_ANY);

/////////////////////////////////////////////////////////////////////////////
unsigned long millis() {
    timeval tv{ };
    gettimeofday(&tv, nullptr);
    return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}

/////////////////////////////////////////////////////////////////////////////
char* utoa(unsigned value, char* result, int base) {
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
char* itoa(int value, char* result, int base) {
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
char* ltoa(long value, char* result, int base) {
    return itoa(value, result, base);
}
char* ultoa(unsigned long value, char* result, int base) {
    return itoa(value, result, base);
}

char* dtostrf(double number, signed char width, unsigned char prec, char *s) {
    sprintf(s, "%lf", number);
    return s;
}

/////////////////////////////////////////////////////////////////////////////
void yield() { }

} // extern "C"

/////////////////////////////////////////////////////////////////////////////
std::ostream& operator<<(std::ostream& outs, const IPAddress& ip) {
    return outs << (int)ip[0] << '.' << (int)ip[1] << '.' << (int)ip[2] << '.' << (int)ip[3];
}
std::ostream& operator<<(std::ostream& outs, const String& str) {
    return outs << str.c_str();
}
