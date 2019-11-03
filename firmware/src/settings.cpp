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
// #include <ip_addr.h>

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
, propTest_{ &propRoot_, "test" }
, propTestInt_{ &propTest_, "int", 42 }
, propPower_{ &propRoot_, "power", 0 }
, propVersion_{ &propRoot_, "version", version::STRING }
, propVersionGit_{ &propRoot_, "gitRev", version::GIT_REV }
, propVoltage_{ &propRoot_, "voltage", 0 }
{ }

/////////////////////////////////////////////////////////////////////////////
void Settings::begin() {
    lastMillisDirty_ = millis();
    lastMillisPersist_ = millis();
}

/////////////////////////////////////////////////////////////////////////////
void Settings::tick() {
    const auto now = millis();

    // process dirty properties
    if ((now - lastMillisDirty_) >= 100) {
        lastMillisDirty_ = now;

        if (onDirtyProperties_ && propRoot_.dirty()) {
            DynamicJsonDocument docProps{Settings::JSON_STATE_SIZE};
            propRoot_.toJson(docProps, Property::DIRTY);
            if (!docProps.isNull()) onDirtyProperties_(docProps);
        }
    }

    // persist properties
    if ((now - lastMillisPersist_) >= 2000) {
        lastMillisPersist_ = now;

        if (onPersistProperties_ && propRoot_.persistDirty()) {
            printf("Saving properties...\r\n");
            DynamicJsonDocument docProps{Settings::JSON_STATE_SIZE};
            propRoot_.toJson(docProps, Property::PERSIST);
            if (!docProps.isNull()) onPersistProperties_(docProps);
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
/// load settings from JSON stream
void Settings::loadFrom(Stream& config) {
    DynamicJsonDocument doc{Settings::JSON_STATE_SIZE};
    deserializeJson(doc, config);
    propRoot_.fromJson(doc.to<JsonObject>());
}

/////////////////////////////////////////////////////////////////////////////
/// update measurements
void Settings::updateMeasurements(double watts, double volts) {
    propPower_.set(watts);
    propVoltage_.set(volts);
}

/////////////////////////////////////////////////////////////////////////////
/// call method
JsonRpcError Settings::call(const char* method, const JsonVariant& params, JsonDocument& result) {
    for (const auto& m : methods_) {
        if (0 != strcmp(method, m.first)) continue;
        return (this->*(m.second))(params, result);
    }

    result.set("Method not found");
    return JsonRpcError::METHOD_NOT_FOUND;
}

/////////////////////////////////////////////////////////////////////////////
/// network - apply new network settings
JsonRpcError Settings::methodNetwork_(const JsonVariant& params, JsonDocument& result) {
    const char* ssid = params["ssid"];
    if (!ssid) { result.set("Missing SSID"); return JsonRpcError::INVALID_PARAMS; }

    NetworkUPtr network(new Network);
    if (!network) { result.set("Allocation failed"); return JsonRpcError::INTERNAL_ERROR; }

    network->ssid = ssid;
    network->hostname = static_cast<const char*>(params["hostname"]);
    network->password = static_cast<const char*>(params["password"]);

    const auto dhcp = params["dhcp"].as<bool>();

    // pick up IP address settings
    // an empty or invalid ipv4Address assumes DHCP
    const char* ip = params["ipv4Address"];
    if (!dhcp && ip) {
        if (!network->ipv4Address.fromString(ip)) { result.set("Invalid ipv4Address"); return JsonRpcError::INVALID_PARAMS; }

        // check IP validity
        if (INADDR_ANY != network->ipv4Address && INADDR_NONE != network->ipv4Address) {
            const char* subnet = params["ipv4Subnet"];
            if (!subnet) { result.set("Missing ipv4Subnet"); return JsonRpcError::INVALID_PARAMS; }
            if (!network->ipv4Subnet.fromString(subnet)) { result.set("Invalid ipv4Subnet"); return JsonRpcError::INVALID_PARAMS; }

            // valid subnet?
            if (!utils::validSubnet(network->ipv4Subnet)) { result.set("Invalid ipv4Subnet"); return JsonRpcError::INVALID_PARAMS; }

            // optional gateway
            const char* gateway = params["ipv4Gateway"];
            if (gateway && gateway[0] && !network->ipv4Gateway.fromString(gateway)) { result.set("Invalid ipv4Gateway"); return JsonRpcError::INVALID_PARAMS; }

            // optional DNS
            const char* dns1 = params["ipv4Dns1"];
            if (dns1 && dns1[0] && !network->ipv4Dns1.fromString(dns1)) { result.set("Invalid ipv4Dns1"); return JsonRpcError::INVALID_PARAMS; }
            const char* dns2 = params["ipv4Dns2"];
            if (dns2 && dns2[0] && !network->ipv4Dns2.fromString(dns2)) { result.set("Invalid ipv4Dns2"); return JsonRpcError::INVALID_PARAMS; }
        }
    }

    // apply settings
    const bool res = !onNetwork_ || onNetwork_(std::move(network));
    result.set(res);
    return JsonRpcError::NO_ERROR;
}

/////////////////////////////////////////////////////////////////////////////
/// ping - responds with pong
JsonRpcError Settings::methodPing_(const JsonVariant& /*params*/, JsonDocument& result) {
    result.set("pong");
    return JsonRpcError::NO_ERROR;
}

/////////////////////////////////////////////////////////////////////////////
/// relay - set relay
JsonRpcError Settings::methodRelay_(const JsonVariant& params, JsonDocument& result) {
    if (!params.is<bool>()) { result.set("Expected boolean"); return JsonRpcError::INVALID_PARAMS; }

    setRelay(params.as<bool>());

    result.set(true);
    return JsonRpcError::NO_ERROR;
}

/////////////////////////////////////////////////////////////////////////////
/// update relay state
void Settings::setRelay(bool state) {
    if (propRelay_.value() != state) {
        propRelay_.set(state);
        if (onRelay_) onRelay_(state);
    }
}

/////////////////////////////////////////////////////////////////////////////
/// state - retrieve current settings
JsonRpcError Settings::methodState_(const JsonVariant& /*params*/, JsonDocument& result) {
    this->toJson(result);
    return JsonRpcError::NO_ERROR;
}

/////////////////////////////////////////////////////////////////////////////
/// test - used for testing the RPC interface
JsonRpcError Settings::methodTest_(const JsonVariant& /*params*/, JsonDocument& result) {
    const auto newValue = propTestInt_.value() + 1;
    propTestInt_.set(newValue);
    result.set(newValue);
    return JsonRpcError::NO_ERROR;
}
