/////////////////////////////////////////////////////////////////////////////
/** @file
Encapsulates a property

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED__PROPERTY
#define INCLUDED__PROPERTY

//- includes
#include <cassert>
#include <IPAddress.h>
#include <utility>
#include <WString.h>
#include <ArduinoJson.h>

//- forwards
class Property;
class PropertyNode;
template <typename T>
class PropertyValueT;

using PropertyBool      = PropertyValueT<bool>;     ///< holds a boolean
using PropertyFloat     = PropertyValueT<float>;    ///< holds a float
using PropertyInt       = PropertyValueT<int>;      ///< holds an integer
using PropertyIpAddress = PropertyValueT<IPAddress>;///< holds an IP address
using PropertyString    = PropertyValueT<String>;   ///< holds a string

/////////////////////////////////////////////////////////////////////////////
/// property encapsulation
class Property {
    /// ick; PropertyNode manages our intrusive doubly linked list
    friend PropertyNode;
public:
    virtual ~Property();

    // noncopyable
    Property(const Property&) = delete;
    // nonassignable
    Property& operator=(const Property&) = delete;

    // toJson flags
    enum JsonFlags {
        JSON_DIRTY          = 1 << 0,   ///< this property or a child property has been modified
        JSON_DIRTY_PERSIST  = 1 << 1,   ///< this property or a child persisted property has been modified
        JSON_PERSIST        = 1 << 2,   ///< persist this property
    };

    /////////////////////////////////////////////////////////////////////////
    /// retrieve name
    const String& name() const { return name_; }

    /////////////////////////////////////////////////////////////////////////
    /// dirty property? (property changed)
    bool dirty() const { return flags_ & JSON_DIRTY; }
    void setDirty();
    virtual void clearDirty();

    /// one or more persistent properties dirty?
    bool persistDirty() const { return flags_ & JSON_DIRTY_PERSIST; }

    /////////////////////////////////////////////////////////////////////////
    /// persist property?
    bool persist() const { return flags_ & JSON_PERSIST; }
    void setPersist();

protected:
    /////////////////////////////////////////////////////////////////////////
    /// constructor
    explicit Property(PropertyNode* parent = nullptr, String name = String{});

    /////////////////////////////////////////////////////////////////////////
    /// convert to JSON
    virtual void toJson_(JsonObject& json, int flags) = 0;

private:
    const String    name_;                      ///< property name
    int             flags_ = 0;                 ///< associated flags

    PropertyNode*   parent_ = nullptr;          ///< our parent property
    Property*       siblingPrev_ = nullptr;     ///< previous sibling
    Property*       siblingNext_ = nullptr;     ///< next sibling
};


/////////////////////////////////////////////////////////////////////////////
/// property parent node
/// splits concept of a property which holds a value (PropertyValueT)
/// and a property which contains other properties (PropertyNode)
class PropertyNode : public Property {
public:
    /////////////////////////////////////////////////////////////////////////
    /// constructor
    explicit PropertyNode(PropertyNode* parent = nullptr, String name = String{})
    : Property(parent, std::move(name))
    { }
    /// destructor
    ~PropertyNode() override = default;

    // noncopyable
    PropertyNode(const PropertyNode&) = delete;
    // nonassignable
    PropertyNode& operator=(const PropertyNode&) = delete;

    void addChild(Property& child);
    void removeChild(Property& child);

    void clearDirty() override;

    void fromJson(const JsonVariant& json);
    JsonObject& toJson(JsonBuffer& buffer, int flags = 0);

private:
    void fromJson_(const JsonVariant& json) override;
    void toJson_(JsonObject& json, int flags) override;
    void jsonChildren_(JsonObject& json, int flags);

    Property*   childFirst_ = nullptr;      ///< first child property
    Property*   childLast_ = nullptr;       ///< last child property
};


/////////////////////////////////////////////////////////////////////////////
/// removed property sentinel
class PropertyZombie : public Property {
public:
    /////////////////////////////////////////////////////////////////////////
    /// constructor
    PropertyZombie(PropertyNode* parent, String name)
    : Property(parent, std::move(name))
    { }
    /// destructor
    ~PropertyZombie() override = default;

protected:
    /////////////////////////////////////////////////////////////////////////
    /// output JSON
    void toJson_(JsonObject& json, int /*flags*/) override {
        json.set(name().c_str(), (const char*)nullptr);
    }
};


/////////////////////////////////////////////////////////////////////////////
/// property holding a type T
template <typename T>
class PropertyValueT : public Property {
public:
    /////////////////////////////////////////////////////////////////////////
    /// constructor
    PropertyValueT(PropertyNode* parent, String name, T value = T{})
    : Property(parent, std::move(name))
    , value_(value)
    { }
    /// destructor
    ~PropertyValueT() override = default;

    /////////////////////////////////////////////////////////////////////////
    /// retrieve underlying value
    const T& value() const { return value_; }

    /////////////////////////////////////////////////////////////////////////
    /// assign new value
    void set(T new_value) {
        if (value_ == new_value) return;
        value_ = new_value;
        setDirty();
    }

protected:
    /////////////////////////////////////////////////////////////////////////
    /// process from JSON
    void fromJson_(const JsonVariant& json) override {
        if (json.is<T>()) {
            value_ = json.as<T>();
        }
    }
    /// output JSON
    void toJson_(JsonObject& json, int /*flags*/) override {
        json.set(name().c_str(), value());
    }

private:
    T   value_{};   ///< held value
};


/////////////////////////////////////////////////////////////////////////////
/// specialize fromJson handling of IPAddress
template <>
inline void PropertyValueT<IPAddress>::fromJson_(const JsonVariant& json) {
    if (json.is<const char*>()) {
        value_.fromString(json.as<String>());
    }
}
/// specialize fromJson handling of String
template <>
inline void PropertyValueT<String>::fromJson_(const JsonVariant& json) {
    if (json.is<const char*>()) {
        value_ = json.as<String>();
    }
}

/////////////////////////////////////////////////////////////////////////////
/// specialize toJson handling of IPAddress
template <>
inline void PropertyValueT<IPAddress>::toJson_(JsonObject& json, int /*flags*/) {
    json.set(name().c_str(), value().toString());
}

#endif // INCLUDED__PROPERTY
