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
#include <functional>

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
    /// response to command request
    using CommandResult = std::pair<JsonRpcError, JsonVariant>;
    /// callback for dirty property notifications
    using FuncOnDirtyProperties = std::function<void (const JsonObject&, JsonBuffer& buffer)>;
    /// callback on relay change
    using FuncOnRelay = std::function<void (bool)>;

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
    /// relay changes
    void onRelay(FuncOnRelay onRelay) {
        onRelay_ = std::move(onRelay);
    }

    /////////////////////////////////////////////////////////////////////////
    void setSsid(String ssid) {
        propSysSsid_.setValue(std::move(ssid));
    }

    CommandResult onCommand(const char* method, const JsonVariant& params, JsonBuffer& buffer);

    void updateMeasurements(double watts, double volts);

private:
    PropertyNode            propRoot_;
    PropertyBool            propRelay_;
    PropertyNode            propSys_;
    PropertyString          propSysSsid_;
    PropertyNode            propTest_;
    PropertyInt             propTestInt_;
    PropertyFloat           propPower_;
    PropertyString          propVersion_;
    PropertyString          propVersionGit_;
    PropertyFloat           propVoltage_;

    FuncOnDirtyProperties   onDirtyProperties_;     ///< on dirty property notification
    unsigned long           lastMillis_{0};         ///< last dirty check

    FuncOnRelay             onRelay_;               ///< on relay

    bool                    need_reboot_{false};    ///< need to perform a reboot
};

#endif // INCLUDED__SETTINGS
