/////////////////////////////////////////////////////////////////////////////
/** @file
Test settings

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////

//- includes
#include "doctest_ext.h"
#include "settings.h"

/////////////////////////////////////////////////////////////////////////////
TEST_SUITE("Settings") {
    /////////////////////////////////////////////////////////////////////////
    TEST_CASE("call - invalid") {
        Settings settings;

        {
            DynamicJsonDocument result{Settings::JSON_STATE_SIZE};
            const auto error = settings.call("unknown", JsonObject{}, result);
            CHECK(error == JsonRpcError::METHOD_NOT_FOUND);
            CHECK(result.as<std::string>() == "Method not found");
        }
    }

    /////////////////////////////////////////////////////////////////////////
    TEST_CASE("call - network") {
        Settings settings;

        Settings::NetworkUPtr networkSettings;
        settings.onNetwork([&networkSettings](Settings::NetworkUPtr&& toApply) {
            networkSettings = std::move(toApply);
            return true;
        });

        DynamicJsonDocument resultDoc{Settings::JSON_STATE_SIZE};
        {
            const auto error = settings.call("network", JsonObject{}, resultDoc);
            CHECK(error == JsonRpcError::INVALID_PARAMS);
            const auto result = resultDoc.as<std::string>();
            CHECK(result == "Missing SSID");
        }
        {
            DynamicJsonDocument param{Settings::JSON_REQUEST_SIZE};
            auto obj = param.to<JsonObject>();
            obj["ssid"]        = "ssid";
            obj["ipv4Address"] = "invalid";

            const auto error = settings.call("network", obj, resultDoc);
            CHECK(error == JsonRpcError::INVALID_PARAMS);
            const auto result = resultDoc.as<std::string>();
            CHECK(result == "Invalid ipv4Address");
        }
        {
            DynamicJsonDocument param{Settings::JSON_REQUEST_SIZE};
            auto obj = param.to<JsonObject>();
            obj["ssid"]        = "ssid";
            obj["ipv4Address"] = "1.2.3.4";

            const auto error = settings.call("network", obj, resultDoc);
            CHECK(error == JsonRpcError::INVALID_PARAMS);
            const auto result = resultDoc.as<std::string>();
            CHECK(result == "Missing ipv4Subnet");
        }
        {
            DynamicJsonDocument param{Settings::JSON_REQUEST_SIZE};
            auto obj = param.to<JsonObject>();
            obj["ssid"]        = "ssid";
            obj["ipv4Address"] = "1.2.3.4";
            obj["ipv4Subnet"]  = "invalid";

            const auto error = settings.call("network", obj, resultDoc);
            CHECK(error == JsonRpcError::INVALID_PARAMS);
            const auto result = resultDoc.as<std::string>();
            CHECK(result == "Invalid ipv4Subnet");
        }
        {
            DynamicJsonDocument param{Settings::JSON_REQUEST_SIZE};
            auto obj = param.to<JsonObject>();
            obj["ssid"]        = "ssid";
            obj["ipv4Address"] = "1.2.3.4";
            obj["ipv4Subnet"]  = "1.2.3.4";

            const auto error = settings.call("network", obj, resultDoc);
            CHECK(error == JsonRpcError::INVALID_PARAMS);
            const auto result = resultDoc.as<std::string>();
            CHECK(result == "Invalid ipv4Subnet");
        }

        {
            DynamicJsonDocument param{Settings::JSON_REQUEST_SIZE};
            auto obj = param.to<JsonObject>();
            obj["ssid"] = "ssid";

            const auto error = settings.call("network", obj, resultDoc);
            CHECK(error == JsonRpcError::NO_ERROR);
            const auto result = resultDoc.as<bool>();
            CHECK(result == true);

            CHECK(networkSettings->hostname == "");
            CHECK(networkSettings->ssid == "ssid");
            CHECK(networkSettings->password == "");
            CHECK(!networkSettings->ipv4Address.isSet());
            CHECK(!networkSettings->ipv4Subnet.isSet());
            CHECK(!networkSettings->ipv4Gateway.isSet());
        }
        {
            DynamicJsonDocument param{Settings::JSON_REQUEST_SIZE};
            auto obj = param.to<JsonObject>();
            obj["ssid"] = "ssid";
            obj["ipv4Address"] = "0.0.0.0";
            obj["ipv4Subnet"]  = "doesn't matter";

            const auto error = settings.call("network", obj, resultDoc);
            CHECK(error  == JsonRpcError::NO_ERROR);
            const auto result = resultDoc.as<bool>();
            CHECK(result == true);

            CHECK(networkSettings->hostname == "");
            CHECK(networkSettings->ssid == "ssid");
            CHECK(networkSettings->password == "");
            CHECK(!networkSettings->ipv4Address.isSet());
            CHECK(!networkSettings->ipv4Subnet.isSet());
            CHECK(!networkSettings->ipv4Gateway.isSet());
        }
        {
            DynamicJsonDocument param{Settings::JSON_REQUEST_SIZE};
            auto obj = param.to<JsonObject>();
            obj["hostname"]    = "hostname";
            obj["ssid"]        = "ssid";
            obj["password"]    = "password";
            obj["ipv4Address"] = "192.168.1.2";
            obj["ipv4Subnet"]  = "255.255.255.0";
            obj["ipv4Gateway"] = "192.168.1.1";

            const auto error = settings.call("network", obj, resultDoc);
            CHECK(error  == JsonRpcError::NO_ERROR);
            const auto result = resultDoc.as<bool>();
            CHECK(result == true);

            CHECK(networkSettings->hostname == "hostname");
            CHECK(networkSettings->ssid     == "ssid");
            CHECK(networkSettings->password == "password");
            CHECK(networkSettings->ipv4Address.toString() == "192.168.1.2");
            CHECK(networkSettings->ipv4Subnet.toString()  == "255.255.255.0");
            CHECK(networkSettings->ipv4Gateway.toString() == "192.168.1.1");
        }
    }

    /////////////////////////////////////////////////////////////////////////
    TEST_CASE("call - ping") {
        Settings settings;

        DynamicJsonDocument resultDoc{2048};
        {
            const auto error = settings.call("ping", JsonObject{}, resultDoc);
            CHECK(error == JsonRpcError::NO_ERROR);
            const auto result = resultDoc.as<std::string>();
            CHECK(result == "pong");
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

        DynamicJsonDocument paramsDoc{1024};
        auto params = paramsDoc.to<JsonVariant>();

        DynamicJsonDocument resultDoc{1024};
        {
            const auto error = settings.call("relay", JsonObject{}, resultDoc);
            CHECK(error == JsonRpcError::INVALID_PARAMS);
            const auto result = resultDoc.as<std::string>();
            CHECK(result == "Expected boolean");
            CHECK(onRelayCalled == 0);
        }

        {
            params.set(true);
            const auto error = settings.call("relay", params, resultDoc);
            CHECK(error == JsonRpcError::NO_ERROR);
            const auto result = resultDoc.as<bool>();
            CHECK(result == true);
            CHECK(onRelayCalled == 1);
            CHECK(relayState == true);
        }
        {
            params.set(true);
            const auto error = settings.call("relay", params, resultDoc);
            CHECK(error == JsonRpcError::NO_ERROR);
            const auto result = resultDoc.as<bool>();
            CHECK(result == true);
            CHECK(onRelayCalled == 1); // <= no change
            CHECK(relayState == true);
        }

        {
            params.set(false);
            const auto error = settings.call("relay", params, resultDoc);
            CHECK(error == JsonRpcError::NO_ERROR);
            const auto result = resultDoc.as<bool>();
            CHECK(result == true);
            CHECK(onRelayCalled == 2);
            CHECK(relayState == false);
        }
        {
            params.set(false);
            const auto error = settings.call("relay", params, resultDoc);
            CHECK(error == JsonRpcError::NO_ERROR);
            const auto result = resultDoc.as<bool>();
            CHECK(result == true);
            CHECK(onRelayCalled == 2); // <= no change
            CHECK(relayState == false);
        }
    }
}
