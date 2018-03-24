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
#include <utility>
#include <WString.h>
#include <ArduinoJson.h>

//- forwards
class Property;
class PropertyNode;
template <typename T>
class PropertyValueT;

using PropertyBool  = PropertyValueT<bool>;     ///< holds a boolean
using PropertyFloat = PropertyValueT<float>;    ///< holds a float
using PropertyInt   = PropertyValueT<int>;      ///< holds an integer

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
        JSON_DIRTY = 1 << 0,
    };


    /////////////////////////////////////////////////////////////////////////
    /// retrieve name
    const String& name() const { return name_; }

    /////////////////////////////////////////////////////////////////////////
    /// dirty property? (property changed)
    bool dirty() const { return dirty_; }
    void markDirty();

protected:
    /////////////////////////////////////////////////////////////////////////
    /// constructor
    explicit Property(PropertyNode* parent = nullptr, String name = String{});

    /////////////////////////////////////////////////////////////////////////
    /// convert to JSON
    virtual void toJson_(JsonObject& json, int flags) = 0;

private:
    const String    name_;                      ///< property name
    bool            dirty_ = false;             ///< property has been modified

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

    JsonObject& toJson(JsonBuffer& buffer, int flags = 0);

private:
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
    void setValue(T new_value) {
        if (value_ == new_value) return;
        value_ = new_value;
        markDirty();
    }

protected:
    /////////////////////////////////////////////////////////////////////////
    /// output JSON
    void toJson_(JsonObject& json, int /*flags*/) override {
        json.set(name().c_str(), value());
    }

private:
    T   value_{};   ///< held value
};

#endif // INCLUDED__PROPERTY
