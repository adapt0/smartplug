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

//- forwards
class Stream;

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
    enum {
        JSON_REQUEST_SIZE   = 512,  ///< how big of a JSON request we can expect
        JSON_STATE_SIZE     = 4096, ///< JSON limit
    };
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

    /// callback for property notifications
    using FuncOnProperties = std::function<void (const JsonDocument)>;
    /// callback on relay change
    using FuncOnRelay = std::function<void (bool)>;
    /// callback on network settings
    using FuncOnNetwork = std::function<bool (NetworkUPtr&&)>;

    Settings();

    void begin();
    void tick();

    void loadFrom(Stream& config);

    /////////////////////////////////////////////////////////////////////////
    /// indicates device needs to be rebooted
    bool needReboot() const { return need_reboot_; }
    /// indicate that we need to perform a reboot
    void setNeedReboot() { need_reboot_ = true; }

    /////////////////////////////////////////////////////////////////////////
    /// output JSON
    void toJson(JsonDocument& doc) {
        propRoot_.toJson(doc);
    }

    /////////////////////////////////////////////////////////////////////////
    /// dirty properties
    void onDirtyProperties(FuncOnProperties onDirtyProperties) {
        onDirtyProperties_ = std::move(onDirtyProperties);
    }
    /// persist properties
    void onPersistProperties(FuncOnProperties onPersistProperties) {
        onPersistProperties_ = std::move(onPersistProperties);
    }
    /// network settings
    void onNetwork(FuncOnNetwork onNetwork) {
        onNetwork_ = std::move(onNetwork);
    }
    /// relay changes
    void onRelay(FuncOnRelay onRelay) {
        onRelay_ = std::move(onRelay);
    }

    /// current relay value
    bool relay() { return propRelay_.value(); }
    void setRelay(bool state);

    /////////////////////////////////////////////////////////////////////////
    /// sys.net
    PropertyNode& propSysNet() { return propSysNet_; }

    JsonRpcError call(const char* method, const JsonVariant& params, JsonDocument& result);

    void updateMeasurements(double watts, double volts);

private:
    /// member function pointer for handling methods
    using MethodFunc = JsonRpcError (Settings::*)(const JsonVariant&, JsonDocument&);
    /// method name to member function binding
    using MethodFuncPair = std::pair<const char*, MethodFunc>;

    /// collection of methods to member functions
    static const MethodFuncPair methods_[];

    JsonRpcError methodNetwork_(const JsonVariant& params, JsonDocument& result);
    JsonRpcError methodPing_(const JsonVariant& params, JsonDocument& result);
    JsonRpcError methodRelay_(const JsonVariant& params, JsonDocument& result);
    JsonRpcError methodState_(const JsonVariant& params, JsonDocument& result);
    JsonRpcError methodTest_(const JsonVariant& params, JsonDocument& result);

    PropertyNode            propRoot_;
    PropertyBool            propRelay_;
    PropertyNode            propSys_;
    PropertyNode            propSysNet_;
    PropertyNode            propTest_;
    PropertyInt             propTestInt_;
    PropertyFloat           propPower_;
    PropertyString          propVersion_;
    PropertyString          propVersionGit_;
    PropertyFloat           propVoltage_;

    FuncOnProperties        onDirtyProperties_;     ///< on dirty property notification
    FuncOnProperties        onPersistProperties_;   ///< on persist property
    unsigned long           lastMillisDirty_{0};    ///< last dirty check
    unsigned long           lastMillisPersist_{0};  ///< last persist check

    FuncOnNetwork           onNetwork_;             ///< on network settings
    FuncOnRelay             onRelay_;               ///< on relay

    bool                    need_reboot_{false};    ///< need to perform a reboot
};

#endif // INCLUDED__SETTINGS
