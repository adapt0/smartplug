/////////////////////////////////////////////////////////////////////////////
/** @file
Encapsulates a property

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////

//- includes
#include "property.h"

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/// Property
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/// constructor
Property::Property(PropertyNode* parent, String name, int flags)
: name_(std::move(name))
, flags_(flags)
{
    if (parent) parent->addChild(*this);
    for (auto it = this; it; it = it->parent_) it->flags_ |= (DIRTY | flags);
}
/// destructor
Property::~Property() {
    if (parent_) parent_->removeChild(*this);
    parent_ = nullptr;
}

/////////////////////////////////////////////////////////////////////////////
/// clear dirty
void Property::clearDirty() {
    flags_ &= ~(DIRTY | DIRTY_PERSIST);
}
/// mark property (+ parents) as dirty
void Property::setDirty() {
    const int flags = DIRTY | (persist() ? DIRTY_PERSIST : 0);
    for (auto it = this; it; it = it->parent_) it->flags_ |= flags;
}

/////////////////////////////////////////////////////////////////////////////
/// set properties (+ parents) persistence
void Property::setPersist() {
    for (auto it = this; it; it = it->parent_) it->flags_ |= PERSIST;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
/// PropertyNode
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/// add child node
void PropertyNode::addChild(Property& child) {
    assert(!child.parent_ && "addChild but already associated with a parent");
    assert(!child.siblingPrev_ && !child.siblingNext_);

    child.parent_ = this;

    child.siblingNext_ = nullptr;
    child.siblingPrev_ = childLast_;
    if (childLast_) childLast_->siblingNext_ = &child;
    childLast_ = &child;

    if (!childFirst_) childFirst_ = &child;
}

/////////////////////////////////////////////////////////////////////////////
/// remove child node
void PropertyNode::removeChild(Property& child) {
    assert((this == child.parent_) && "removeChild but properties parent differs during removal");

    if (child.siblingPrev_) child.siblingPrev_->siblingNext_ = child.siblingNext_;
    if (child.siblingNext_) child.siblingNext_->siblingPrev_ = child.siblingPrev_;
    if (&child == childFirst_) childFirst_ = child.siblingNext_;
    if (&child == childLast_)  childLast_  = child.siblingPrev_;

    child.parent_ = nullptr;
}

/////////////////////////////////////////////////////////////////////////////
/// clear dirty
void PropertyNode::clearDirty() {
    Property::clearDirty();
    for (auto it = childFirst_; it; it = it->siblingNext_) {
        it->clearDirty();
    }
}

/////////////////////////////////////////////////////////////////////////////
/// load data from JSON
void PropertyNode::fromJson(const JsonVariant& json) {
    return fromJson_(json);
}
/// load data from JSON
void PropertyNode::fromJson_(const JsonVariant& json) {
    for (auto child = childFirst_; child; child = child->siblingNext_) {
        if (child->persist()) {
            child->fromJson_( json[child->name()] );
        }
    }
}

/////////////////////////////////////////////////////////////////////////////
/// visit our property nodes
JsonObject& PropertyNode::toJson(JsonBuffer& buffer, int flags) {
    auto& obj = buffer.createObject();
    if ((flags & DIRTY)   && (flags_ & DIRTY)) flags_ &= ~DIRTY;
    if ((flags & PERSIST) && (flags_ & DIRTY_PERSIST)) flags_ &= ~DIRTY_PERSIST;
    jsonChildren_(obj, flags);
    return obj;
}
/// convert child properties to JSON
void PropertyNode::toJson_(JsonObject& json, int flags) {
    auto& obj = json.createNestedObject(name());
    jsonChildren_(obj, flags);
}
/// populate JSON with children
void PropertyNode::jsonChildren_(JsonObject& json, int flags) {
    for (auto child = childFirst_; child; child = child->siblingNext_) {
        // filter + clear dirty
        if (flags & DIRTY) {
            if (!child->dirty()) continue; // skip non-dirty nodes
            child->flags_ &= ~DIRTY;
        }

        // filter persistent properties
        if (flags & PERSIST) {
            if (!child->persist()) continue; // skip non-persistent nodes
            child->flags_ &= ~DIRTY_PERSIST;
        }

        //
        child->toJson_(json, flags);
    }
}
