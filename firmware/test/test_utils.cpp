/////////////////////////////////////////////////////////////////////////////
/** @file
Test utils

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////

//- includes
#include "doctest_ext.h"
#include "utils.h"
#include <IPAddress.h>

/////////////////////////////////////////////////////////////////////////////
TEST_SUITE("utils") {
    /////////////////////////////////////////////////////////////////////////
    TEST_CASE("validSubnet") {
        CHECK(  utils::validSubnet(IPAddress(128,   0,   0,   0)) );
        CHECK(  utils::validSubnet(IPAddress(255,   0,   0,   0)) );
        CHECK(  utils::validSubnet(IPAddress(255, 255,   0,   0)) );
        CHECK(  utils::validSubnet(IPAddress(255, 255, 255,   0)) );
        CHECK(  utils::validSubnet(IPAddress(255, 255, 255, 254)) );

        CHECK( !utils::validSubnet(IPAddress(0,     0,   0,   0)) );
        CHECK( !utils::validSubnet(IPAddress(0,     0,   0, 255)) );
        CHECK( !utils::validSubnet(IPAddress(0,     0, 255, 255)) );
        CHECK( !utils::validSubnet(IPAddress(0,   255, 255, 255)) );
        CHECK( !utils::validSubnet(IPAddress(255, 255, 255, 255)) );

        CHECK( !utils::validSubnet(IPAddress(  1,   2,   3,   4)) );
        CHECK( !utils::validSubnet(IPAddress(255,   0,   0,   1)) );
        CHECK( !utils::validSubnet(IPAddress(255, 255,   0,   2)) );
    }
}
