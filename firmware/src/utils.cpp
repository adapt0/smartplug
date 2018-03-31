/////////////////////////////////////////////////////////////////////////////
/** @file
General utility functions

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////

//- includes
#include "utils.h"
#include <IPAddress.h>

using namespace utils;

/////////////////////////////////////////////////////////////////////////////
/// check if an IPAddress is a valid subnet mask
/// @returns true if valid
bool utils::validSubnet(const IPAddress& subnet) {
    // convert to Little Endian so we can bit flip
    // 255.255.0.0 => 0xFFFF0000
    uint32_t s = (
           subnet[3]
        | (subnet[2] << 8)
        | (subnet[1] << 16)
        | (subnet[0] << 24)
    );

    // ~ => 0x0000FFFF
    s = ~s;
    if (0 == s) return false;

    // +1 => 0x00010000
    s += 1;

    // http://www.graphics.stanford.edu/~seander/bithacks.html#DetermineIfPowerOf2
    return (s && !(s & (s - 1)));
}
