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
Property::Property(PropertyNode* parent, String name)
: name_(std::move(name))
{
    if (parent) parent->addChild(*this);
    markDirty();
}
/// destructor
Property::~Property() {
    if (parent_) parent_->removeChild(*this);
    parent_ = nullptr;
}

/////////////////////////////////////////////////////////////////////////////
/// mark property (+ parents) as dirty
void Property::markDirty() {
    dirty_ = true;
    for (auto it = parent_; it; it = it->parent_) it->dirty_ = true;
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
/// visit our property nodes
JsonObject& PropertyNode::toJson(JsonBuffer& buffer, int flags) {
    auto& obj = buffer.createObject();
    if (flags & JSON_DIRTY && dirty_) dirty_ = false;
    jsonChildren_(obj, flags);
    return obj;
}
/// convert child properties to JSON
void PropertyNode::toJson_(JsonObject& json, int flags) {
    auto& obj = json.createNestedObject(name().c_str());
    jsonChildren_(obj, flags);
}
/// fill JSON with children
void PropertyNode::jsonChildren_(JsonObject& json, int flags) {
    for (auto child = childFirst_; child; child = child->siblingNext_) {
        // filter + clear dirty
        if (flags & JSON_DIRTY) {
            if (!child->dirty_) continue;
            child->dirty_ = false;
        }

        //
        child->toJson_(json, flags);
    }
}
