/////////////////////////////////////////////////////////////////////////////
/** @file
Settings

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////

//- includes
#include "settings.h"
#include "utils.h"
#include "version.h"
#include <ip_addr.h>

extern "C" unsigned long millis();

/// command methods to function map
const Settings::MethodFuncPair Settings::methods_[] = {
    { "network", &Settings::methodNetwork_ },
    { "ping",    &Settings::methodPing_    },
    { "relay",   &Settings::methodRelay_   },
    { "state",   &Settings::methodState_   },
    { "test",    &Settings::methodTest_    },
};

/////////////////////////////////////////////////////////////////////////////
Settings::Settings()
: propRelay_{ &propRoot_, "relay" }
, propSys_{ &propRoot_, "sys" }
, propSysNet_{ &propSys_, "net" }
, propSysNetHostname_{ &propSysNet_, "hostname" }
, propSysNetSsid_{ &propSysNet_, "ssid" }
, propSysNetDhcp_{ &propSysNet_, "dhcp", true }
, propSysNetIpv4_{ &propSysNet_ }
, propSysNetCur_{ &propSysNet_, "cur" }
, propSysNetCurIpv4_{ &propSysNetCur_ }
, propTest_{ &propRoot_, "test" }
, propTestInt_{ &propTest_, "int", 42 }
, propPower_{ &propRoot_, "power" }
, propVersion_{ &propRoot_, "version", version::STRING }
, propVersionGit_{ &propRoot_, "gitRev", version::GIT_REV }
, propVoltage_{ &propRoot_, "voltage" }
{ }

/////////////////////////////////////////////////////////////////////////////
void Settings::begin() {
    lastMillis_ = millis();
}

/////////////////////////////////////////////////////////////////////////////
void Settings::tick() {
    const auto now = millis();
    if ((now - lastMillis_) < 100) return;
    lastMillis_ = now;

    // process dirty properties
    if (onDirtyProperties_ && propRoot_.dirty()) {
        DynamicJsonBuffer buffer;
        const auto& obj = propRoot_.toJson(buffer, Property::JSON_DIRTY);
        onDirtyProperties_(obj, buffer);
    }
}

/////////////////////////////////////////////////////////////////////////////
/// update measurements
void Settings::updateMeasurements(double watts, double volts) {
    propPower_.set(watts);
    propVoltage_.set(volts);
}

/////////////////////////////////////////////////////////////////////////////
/// call method
Settings::Result Settings::call(const char* method, const JsonVariant& params, JsonBuffer& buffer) {
    for (const auto& m : methods_) {
        if (0 != strcmp(method, m.first)) continue;
        return (this->*(m.second))(params, buffer);
    }
    return Result{JsonRpcError::METHOD_NOT_FOUND, "Method not found"};
}

/////////////////////////////////////////////////////////////////////////////
/// network - apply new network settings
auto Settings::methodNetwork_(const JsonVariant& params, JsonBuffer& /*buffer*/) -> Result {
    const char* ssid = params["ssid"];
    if (!ssid) return Result{JsonRpcError::INVALID_PARAMS, "Missing SSID"};

    NetworkUPtr network(new Network);
    if (!network) return Result{JsonRpcError::INTERNAL_ERROR, "Allocation failed"};

    network->ssid = ssid;
    network->hostname = static_cast<const char*>(params["hostname"]);
    network->password = static_cast<const char*>(params["password"]);

    const auto dhcp = params["dhcp"].as<bool>();

    // pick up IP address settings
    // an empty or invalid ipv4Address assumes DHCP
    const char* ip = params["ipv4Address"];
    if (!dhcp && ip) {
        if (!network->ipv4Address.fromString(ip)) return Result{JsonRpcError::INVALID_PARAMS, "Invalid ipv4Address"};

        // check IP validity
        if (INADDR_NONE != network->ipv4Address) {
            const char* subnet = params["ipv4Subnet"];
            if (!subnet) return Result{JsonRpcError::INVALID_PARAMS, "Missing ipv4Subnet"};
            if (!network->ipv4Subnet.fromString(subnet)) return Result{JsonRpcError::INVALID_PARAMS, "Invalid ipv4Subnet"};

            // valid subnet?
            if (!utils::validSubnet(network->ipv4Subnet)) return Result{JsonRpcError::INVALID_PARAMS, "Invalid ipv4Subnet"};

            // optional gateway
            const char* gateway = params["ipv4Gateway"];
            if (gateway && gateway[0] && !network->ipv4Gateway.fromString(gateway)) return Result{JsonRpcError::INVALID_PARAMS, "Invalid ipv4Gateway"};

            // optional DNS
            const char* dns1 = params["ipv4Dns1"];
            if (dns1 && dns1[0] && !network->ipv4Dns1.fromString(dns1)) return Result{JsonRpcError::INVALID_PARAMS, "Invalid ipv4Dns1"};
            const char* dns2 = params["ipv4Dns2"];
            if (dns2 && dns2[0] && !network->ipv4Dns2.fromString(dns2)) return Result{JsonRpcError::INVALID_PARAMS, "Invalid ipv4Dns2"};
        }
    }

    // populate our properties with the updated settings
    propSysNetDhcp_.set(dhcp || INADDR_NONE == network->ipv4Address);
    if (!dhcp) propSysNetIpv4_.set(*network);

    // apply settings
    const bool res = !onNetwork_ || onNetwork_(std::move(network));
    return Result{JsonRpcError::NO_ERROR, res};
}

/////////////////////////////////////////////////////////////////////////////
/// ping - responds with pong
auto Settings::methodPing_(const JsonVariant& /*params*/, JsonBuffer& /*buffer*/) -> Result {
    return Result{JsonRpcError::NO_ERROR, "pong"};
}

/////////////////////////////////////////////////////////////////////////////
/// relay - set relay
auto Settings::methodRelay_(const JsonVariant& params, JsonBuffer& /*buffer*/) -> Result {
    if (!params.is<bool>()) return Result{JsonRpcError::INVALID_PARAMS, "Expected boolean"};

    const auto newValue = params.as<bool>();
    if (propRelay_.value() != newValue) {
        propRelay_.set(newValue);
        if (onRelay_) onRelay_(newValue);
    }
    return Result{JsonRpcError::NO_ERROR, true};
}

/////////////////////////////////////////////////////////////////////////////
/// state - retrieve current settings
auto Settings::methodState_(const JsonVariant& /*params*/, JsonBuffer& buffer) -> Result {
    return Result{JsonRpcError::NO_ERROR, this->toJson(buffer)};
}

/////////////////////////////////////////////////////////////////////////////
/// test - used for testing the RPC interface
auto Settings::methodTest_(const JsonVariant& /*params*/, JsonBuffer& /*buffer*/) -> Result {
    const auto newValue = propTestInt_.value() + 1;
    propTestInt_.set(newValue);
    return Result{JsonRpcError::NO_ERROR, newValue};
}
