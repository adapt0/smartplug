/////////////////////////////////////////////////////////////////////////////
/** @file
Settings

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED__SETTINGS
#define INCLUDED__SETTINGS

//- includes
#include "property.h"
#include <IPAddress.h>
#include <functional>
#include <memory>

/// JSON-RPC error codes
enum class JsonRpcError {
    NO_ERROR            = 0,        ///< indicates no error
    // error codes from and including -32768 to -32000 are reserved for pre-defined errors
    PARSE_ERROR         = -32700,   ///< Parse error - Invalid JSON was received by the server. An error occurred on the server while parsing the JSON text.
    INVALID_REQUEST     = -32600,   ///< Invalid Request - The JSON sent is not a valid Request object
    METHOD_NOT_FOUND    = -32601,   ///< Method not found - The method does not exist / is not available
    INVALID_PARAMS      = -32602,   ///< Invalid params - Invalid method parameter(s)
    INTERNAL_ERROR      = -32603,   ///< Internal error - Internal JSON-RPC error
    // SERVER_ERROR = -32000 to -32099    Server error    Reserved for implementation-defined server-errors.
};

/////////////////////////////////////////////////////////////////////////////
/// persistent settings
class Settings {
public:
    /// network settings to apply
    struct Network {
        String      hostname;
        String      ssid;
        String      password;
        IPAddress   ipv4Address;
        IPAddress   ipv4Subnet;
        IPAddress   ipv4Gateway;
        IPAddress   ipv4Dns1;
        IPAddress   ipv4Dns2;
    };
    /// pointer to Network
    using NetworkUPtr = std::unique_ptr<Network>;

    /// response to method call
    using Result = std::pair<JsonRpcError, JsonVariant>;
    /// callback for dirty property notifications
    using FuncOnDirtyProperties = std::function<void (const JsonObject&, JsonBuffer& buffer)>;
    /// callback on relay change
    using FuncOnRelay = std::function<void (bool)>;
    /// callback on network settings
    using FuncOnNetwork = std::function<bool (NetworkUPtr&&)>;

    Settings();

    void begin();
    void tick();

    /////////////////////////////////////////////////////////////////////////
    /// indicates device needs to be rebooted
    bool needReboot() const { return need_reboot_; }
    /// indicate that we need to perform a reboot
    void setNeedReboot() { need_reboot_ = true; }

    /////////////////////////////////////////////////////////////////////////
    /// output JSON
    JsonObject& toJson(JsonBuffer& buffer) {
        return propRoot_.toJson(buffer);
    }

    /////////////////////////////////////////////////////////////////////////
    /// dirty properties
    void onDirtyProperties(FuncOnDirtyProperties onDirtyProperties) {
        propRoot_.clearDirty();
        onDirtyProperties_ = std::move(onDirtyProperties);
    }
    /// network settings
    void onNetwork(FuncOnNetwork onNetwork) {
        onNetwork_ = std::move(onNetwork);
    }
    /// relay changes
    void onRelay(FuncOnRelay onRelay) {
        onRelay_ = std::move(onRelay);
    }

    /////////////////////////////////////////////////////////////////////////
    /// update network settings
    void updateNetwork(Network&& network) {
        propSysNetHostname_.set(std::move(network.hostname));
        propSysNetSsid_.set(std::move(network.ssid));
        propSysNetCurIpv4_.set(network);
    }

    Result call(const char* method, const JsonVariant& params, JsonBuffer& buffer);

    void updateMeasurements(double watts, double volts);

private:
    /// member function pointer for handling methods
    using MethodFunc = Result (Settings::*)(const JsonVariant&, JsonBuffer&);
    /// method name to member function binding
    using MethodFuncPair = std::pair<const char*, MethodFunc>;

    /// collection of methods to member functions
    static const MethodFuncPair methods_[];

    Result methodNetwork_(const JsonVariant& params, JsonBuffer& buffer);
    Result methodPing_(const JsonVariant& params, JsonBuffer& buffer);
    Result methodRelay_(const JsonVariant& params, JsonBuffer& buffer);
    Result methodState_(const JsonVariant& params, JsonBuffer& buffer);
    Result methodTest_(const JsonVariant& params, JsonBuffer& buffer);


    /////////////////////////////////////////////////////////////////////////
    /// collection of IPv4 address properties
    struct Ipv4Properties {
        explicit Ipv4Properties(PropertyNode* parent)
        : address{ parent, "ipv4Address" }
        , subnet{ parent, "ipv4Subnet" }
        , gateway{ parent, "ipv4Gateway" }
        , dns1{ parent, "ipv4Dns1" }
        , dns2{ parent, "ipv4Dns2" }
        { }

        /// update properties from Network settings
        void set(const Network& network) {
            address.set(network.ipv4Address);
            subnet.set(network.ipv4Subnet);
            gateway.set(network.ipv4Gateway);
            dns1.set(network.ipv4Dns1);
            dns2.set(network.ipv4Dns2);
        }

        PropertyIpAddress   address;
        PropertyIpAddress   subnet;
        PropertyIpAddress   gateway;
        PropertyIpAddress   dns1;
        PropertyIpAddress   dns2;
    };


    PropertyNode            propRoot_;
    PropertyBool            propRelay_;
    PropertyNode            propSys_;
    PropertyNode            propSysNet_;
    PropertyString          propSysNetHostname_;
    PropertyString          propSysNetSsid_;
    PropertyBool            propSysNetDhcp_;
    Ipv4Properties          propSysNetIpv4_;
    PropertyNode            propSysNetCur_;
    Ipv4Properties          propSysNetCurIpv4_;
    PropertyNode            propTest_;
    PropertyInt             propTestInt_;
    PropertyFloat           propPower_;
    PropertyString          propVersion_;
    PropertyString          propVersionGit_;
    PropertyFloat           propVoltage_;

    FuncOnDirtyProperties   onDirtyProperties_;     ///< on dirty property notification
    unsigned long           lastMillis_{0};         ///< last dirty check

    FuncOnNetwork           onNetwork_;             ///< on network settings
    FuncOnRelay             onRelay_;               ///< on relay

    bool                    need_reboot_{false};    ///< need to perform a reboot
};

#endif // INCLUDED__SETTINGS
