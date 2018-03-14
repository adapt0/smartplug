/////////////////////////////////////////////////////////////////////////////
/** @file
Test settings

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////

//- includes
#include <doctest/doctest.h>
#include "settings.h"

/////////////////////////////////////////////////////////////////////////////
TEST_SUITE("Settings") {
    /////////////////////////////////////////////////////////////////////////
    TEST_CASE("onCommand") {
        Settings settings;

        {
            DynamicJsonBuffer buffer;
            const auto result = settings.onCommand("ping", buffer.createObject(), buffer);
            CHECK(result.first == JsonRpcError::NO_ERROR);
            CHECK(result.second == "pong");
        }

        {
            DynamicJsonBuffer buffer;
            const auto result = settings.onCommand("unknown", buffer.createObject(), buffer);
            CHECK(result.first == JsonRpcError::METHOD_NOT_FOUND);
            CHECK(result.second == "Method not found");
        }
    }
}
