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
    TEST_CASE("call - invalid") {
        Settings settings;

        {
            DynamicJsonBuffer buffer;
            const auto result = settings.call("unknown", buffer.createObject(), buffer);
            CHECK(result.first == JsonRpcError::METHOD_NOT_FOUND);
            CHECK(result.second == "Method not found");
        }
    }

    /////////////////////////////////////////////////////////////////////////
    TEST_CASE("call - network") {
        Settings settings;

        Settings::Network networkSettings;
        settings.onNetwork([&networkSettings](const Settings::Network& toApply) {
            networkSettings = toApply;
            return true;
        });

        {
            DynamicJsonBuffer buffer;
            const auto result = settings.call("network", buffer.createObject(), buffer);
            CHECK(result.first == JsonRpcError::INVALID_PARAMS);
            CHECK(result.second == "Missing SSID");
        }
        {
            DynamicJsonBuffer buffer;
            auto& obj = buffer.createObject();
            obj["ssid"]        = "ssid";
            obj["ipv4Address"] = "invalid";

            const auto result = settings.call("network", obj, buffer);
            CHECK(result.first == JsonRpcError::INVALID_PARAMS);
            CHECK(result.second == "Invalid ipv4Address");
        }
        {
            DynamicJsonBuffer buffer;
            auto& obj = buffer.createObject();
            obj["ssid"]        = "ssid";
            obj["ipv4Address"] = "1.2.3.4";

            const auto result = settings.call("network", obj, buffer);
            CHECK(result.first == JsonRpcError::INVALID_PARAMS);
            CHECK(result.second == "Missing ipv4Subnet");
        }
        {
            DynamicJsonBuffer buffer;
            auto& obj = buffer.createObject();
            obj["ssid"]        = "ssid";
            obj["ipv4Address"] = "1.2.3.4";
            obj["ipv4Subnet"]  = "invalid";

            const auto result = settings.call("network", obj, buffer);
            CHECK(result.first == JsonRpcError::INVALID_PARAMS);
            CHECK(result.second == "Invalid ipv4Subnet");
        }
        {
            DynamicJsonBuffer buffer;
            auto& obj = buffer.createObject();
            obj["ssid"]        = "ssid";
            obj["ipv4Address"] = "1.2.3.4";
            obj["ipv4Subnet"]  = "1.2.3.4";

            const auto result = settings.call("network", obj, buffer);
            CHECK(result.first == JsonRpcError::INVALID_PARAMS);
            CHECK(result.second == "Invalid ipv4Subnet");
        }

        {
            DynamicJsonBuffer buffer;
            auto& obj = buffer.createObject();
            obj["ssid"] = "ssid";

            const auto result = settings.call("network", obj, buffer);
            CHECK(result.first == JsonRpcError::NO_ERROR);
            CHECK(result.second == true);

            CHECK(networkSettings.hostname == "");
            CHECK(networkSettings.ssid == "ssid");
            CHECK(networkSettings.password == "");
            CHECK(networkSettings.ipv4Address.toString() == "0.0.0.0");
            CHECK(networkSettings.ipv4Subnet.toString()  == "0.0.0.0");
            CHECK(networkSettings.ipv4Gateway.toString() == "0.0.0.0");
        }
        {
            DynamicJsonBuffer buffer;
            auto& obj = buffer.createObject();
            obj["ssid"] = "ssid";
            obj["ipv4Address"] = "0.0.0.0";
            obj["ipv4Subnet"]  = "doesn't matter";

            const auto result = settings.call("network", obj, buffer);
            CHECK(result.first == JsonRpcError::NO_ERROR);
            CHECK(result.second == true);

            CHECK(networkSettings.hostname == "");
            CHECK(networkSettings.ssid == "ssid");
            CHECK(networkSettings.password == "");
            CHECK(networkSettings.ipv4Address.toString() == "0.0.0.0");
            CHECK(networkSettings.ipv4Subnet.toString()  == "0.0.0.0");
            CHECK(networkSettings.ipv4Gateway.toString() == "0.0.0.0");
        }
        {
            DynamicJsonBuffer buffer;
            auto& obj = buffer.createObject();
            obj["hostname"]    = "hostname";
            obj["ssid"]        = "ssid";
            obj["password"]    = "password";
            obj["ipv4Address"] = "192.168.1.2";
            obj["ipv4Subnet"]  = "255.255.255.0";
            obj["ipv4Gateway"] = "192.168.1.1";

            const auto result = settings.call("network", obj, buffer);
            CHECK(result.first == JsonRpcError::NO_ERROR);
            CHECK(result.second == true);

            CHECK(networkSettings.hostname == "hostname");
            CHECK(networkSettings.ssid     == "ssid");
            CHECK(networkSettings.password == "password");
            CHECK(networkSettings.ipv4Address.toString() == "192.168.1.2");
            CHECK(networkSettings.ipv4Subnet.toString()  == "255.255.255.0");
            CHECK(networkSettings.ipv4Gateway.toString() == "192.168.1.1");
        }
    }

    /////////////////////////////////////////////////////////////////////////
    TEST_CASE("call - ping") {
        Settings settings;

        {
            DynamicJsonBuffer buffer;
            const auto result = settings.call("ping", buffer.createObject(), buffer);
            CHECK(result.first == JsonRpcError::NO_ERROR);
            CHECK(result.second == "pong");
        }
    }

    /////////////////////////////////////////////////////////////////////////
    TEST_CASE("call - relay") {
        Settings settings;

        int onRelayCalled = 0;
        bool relayState = false;
        settings.onRelay([&](bool state) {
            relayState = state;
            onRelayCalled++;
        });

        {
            DynamicJsonBuffer buffer;
            const auto result = settings.call("relay", buffer.createObject(), buffer);
            CHECK(result.first == JsonRpcError::INVALID_PARAMS);
            CHECK(result.second == "Expected boolean");
            CHECK(onRelayCalled == 0);
        }

        {
            DynamicJsonBuffer buffer;
            const auto result = settings.call("relay", true, buffer);
            CHECK(result.first == JsonRpcError::NO_ERROR);
            CHECK(result.second == true);
            CHECK(onRelayCalled == 1);
            CHECK(relayState == true);
        }
        {
            DynamicJsonBuffer buffer;
            const auto result = settings.call("relay", true, buffer);
            CHECK(result.first == JsonRpcError::NO_ERROR);
            CHECK(result.second == true);
            CHECK(onRelayCalled == 1); // <= no change
            CHECK(relayState == true);
        }

        {
            DynamicJsonBuffer buffer;
            const auto result = settings.call("relay", false, buffer);
            CHECK(result.first == JsonRpcError::NO_ERROR);
            CHECK(result.second == true);
            CHECK(onRelayCalled == 2);
            CHECK(relayState == false);
        }
        {
            DynamicJsonBuffer buffer;
            const auto result = settings.call("relay", false, buffer);
            CHECK(result.first == JsonRpcError::NO_ERROR);
            CHECK(result.second == true);
            CHECK(onRelayCalled == 2); // <= no change
            CHECK(relayState == false);
        }
    }
}
