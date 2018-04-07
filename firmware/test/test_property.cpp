/////////////////////////////////////////////////////////////////////////////
/** @file
Test property

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////

//- includes
#include <doctest/doctest.h>
#include "property.h"
#include <string>

/////////////////////////////////////////////////////////////////////////////
/// convert property tree to a JSON string
std::string toJson(PropertyNode& property, int flags = 0) {
    DynamicJsonBuffer jsonBuffer;
    std::string out;
    property.toJson(jsonBuffer, flags).printTo(out);
    return out;
}

/////////////////////////////////////////////////////////////////////////////
TEST_SUITE("Property") {
    /////////////////////////////////////////////////////////////////////////
    TEST_CASE("toJson") {
        PropertyNode root;
        CHECK(String{} == root.name());
        CHECK(root.dirty());
        CHECK(toJson(root) == R"({})");

        {
            PropertyNode prop_parent{ &root, "parent" };
            CHECK(prop_parent.name() == "parent");
            CHECK(prop_parent.dirty());
            CHECK(toJson(root) == R"({"parent":{}})");

            PropertyInt prop_child{ &root, "child", 0 };
            CHECK(prop_child.name() == "child");
            CHECK(prop_child.value() == 0);
            CHECK(prop_child.dirty());
            CHECK(toJson(root) == R"({"parent":{},"child":0})");

            {
                PropertyBool prop_parent_child1{ &prop_parent, "bool", false };
                CHECK(prop_parent_child1.name() == "bool");
                CHECK(prop_parent_child1.value() == false);
                CHECK(prop_parent_child1.dirty());
                CHECK(toJson(root) == R"({"parent":{"bool":false},"child":0})");

                prop_parent_child1.set(true);
                CHECK(prop_parent_child1.value() == true);
                CHECK(prop_parent_child1.dirty());
                CHECK(toJson(root) == R"({"parent":{"bool":true},"child":0})");

                {
                    PropertyInt prop_parent_child2{ &prop_parent, "int", 314 };
                    CHECK(prop_parent_child2.name() == "int");
                    CHECK(prop_parent_child2.value() == 314);
                    CHECK(prop_parent_child2.dirty());
                    CHECK(toJson(root) == R"({"parent":{"bool":true,"int":314},"child":0})");

                    prop_parent_child2.set(123);
                    CHECK(prop_parent_child2.value() == 123);
                    CHECK(prop_parent_child2.dirty());
                    CHECK(toJson(root) == R"({"parent":{"bool":true,"int":123},"child":0})");
                }

                {
                    PropertyIpAddress prop_parent_child2{ &prop_parent, "ip", IPAddress{192, 168, 1, 100} };
                    CHECK(prop_parent_child2.name() == "ip");
                    CHECK(prop_parent_child2.value() == IPAddress{192, 168, 1, 100});
                    CHECK(prop_parent_child2.dirty());
                    CHECK(toJson(root) == R"({"parent":{"bool":true,"ip":"192.168.1.100"},"child":0})");
                }

                CHECK(toJson(root) == R"({"parent":{"bool":true},"child":0})");
            }
            CHECK(toJson(root) == R"({"parent":{},"child":0})");
        }
        CHECK(toJson(root) == R"({})");
    }

    /////////////////////////////////////////////////////////////////////////
    TEST_CASE("toJson dirty") {
        PropertyNode root;
        CHECK(root.dirty());
        CHECK(toJson(root, Property::JSON_DIRTY) == R"({})"); // empty
        CHECK(false == root.dirty());

        {
            PropertyNode parent{ &root, "parent" };
            CHECK(root.dirty());
            CHECK(toJson(root, Property::JSON_DIRTY) == R"({"parent":{}})");
            CHECK(false == root.dirty());
            CHECK(toJson(root, Property::JSON_DIRTY) == R"({})"); // no change
            CHECK(false == root.dirty());

            PropertyInt child{ &parent, "child", 123 };
            CHECK(root.dirty());
            CHECK(toJson(root, Property::JSON_DIRTY) == R"({"parent":{"child":123}})");
            CHECK(false == root.dirty());
            CHECK(toJson(root, Property::JSON_DIRTY) == R"({})"); // no changes
            CHECK(false == root.dirty());

            {
                PropertyInt child2{ &parent, "child2", 234 };
                CHECK(root.dirty());
                CHECK(toJson(root, Property::JSON_DIRTY) == R"({"parent":{"child2":234}})");
                CHECK(toJson(root, Property::JSON_DIRTY) == R"({})"); // no changes
                CHECK(false == root.dirty());

                child.set(345);
                CHECK(root.dirty());
                CHECK(toJson(root, Property::JSON_DIRTY) == R"({"parent":{"child":345}})");
                CHECK(false == root.dirty());

                child.set(1);
                child2.set(2);
                CHECK(root.dirty());
                CHECK(toJson(root, Property::JSON_DIRTY) == R"({"parent":{"child":1,"child2":2}})");
                CHECK(false == root.dirty());
            }

            // child2 removed
            // CHECK(root.dirty());
            // CHECK(toJson(root, Property::JSON_DIRTY) == R"({"parent":{"child2":null}})"); // removed
            // CHECK(false == root.dirty());
        }

        // parent + child removed
        // CHECK(root.dirty());
        // CHECK(toJson(root, Property::JSON_DIRTY) == R"({"parent":null})"); // removed
        // CHECK(false == root.dirty());
    }

    /////////////////////////////////////////////////////////////////////////
    TEST_CASE("toJson dirty persistence") {
        PropertyNode root;
        CHECK(toJson(root, Property::JSON_PERSIST) == R"({})"); // empty

        PropertyNode prop_parent{ &root, "parent" };
        PropertyInt prop_child1{ &prop_parent, "child1", 1 };
        PropertyInt prop_child2{ &prop_parent, "child2", 2 };
        CHECK(root.dirty());
        CHECK(false == root.persistDirty());

        // grabbing persistent properties does not clear dirty flag
        CHECK(toJson(root, Property::JSON_PERSIST) == R"({})");
        CHECK(root.dirty());
        CHECK(false == root.persistDirty());

        // retrieve dirty clears dirty
        CHECK(toJson(root, Property::JSON_DIRTY) == R"({"parent":{"child1":1,"child2":2}})");
        CHECK(false == root.dirty());
        CHECK(false == root.persistDirty());

        // setting persistent doesn't mark dirty
        prop_child2.setPersist();
        CHECK(false == root.dirty());
        CHECK(false == root.persistDirty());

        // can retrieve persisted nodes
        CHECK(toJson(root, Property::JSON_PERSIST) == R"({"parent":{"child2":2}})");

        // setting value marks dirty
        prop_child2.set(-2);
        CHECK(root.dirty());
        CHECK(root.persistDirty());

        // retrieve persisted clears persisted dirty
        CHECK(toJson(root, Property::JSON_PERSIST) == R"({"parent":{"child2":-2}})");
        CHECK(root.dirty());
        CHECK(false == root.persistDirty());

        CHECK(toJson(root, Property::JSON_DIRTY) == R"({"parent":{"child2":-2}})");
        CHECK(false == root.dirty());
        CHECK(false == root.persistDirty());


        // add another persistent property
        PropertyInt prop_child3{ &prop_parent, "child3", 3 };
        prop_child3.setPersist();
        CHECK(root.dirty());            // dirty because we added a new node
        CHECK(false == root.persistDirty());

        CHECK(toJson(root, Property::JSON_DIRTY) == R"({"parent":{"child3":3}})");
        CHECK(false == root.dirty());
        CHECK(false == root.persistDirty());

        CHECK(toJson(root, Property::JSON_PERSIST) == R"({"parent":{"child2":-2,"child3":3}})");
        CHECK(false == root.dirty());
        CHECK(false == root.persistDirty());


        // change non-persisted node
        prop_child1.set(-1);
        CHECK(root.dirty());
        CHECK(false == root.persistDirty());
        CHECK(toJson(root, Property::JSON_DIRTY) == R"({"parent":{"child1":-1}})");
        CHECK(false == root.dirty());
        CHECK(false == root.persistDirty());

        // persisted node
        prop_child3.set(-3);
        CHECK(root.dirty());
        CHECK(root.persistDirty());
        CHECK(toJson(root, Property::JSON_DIRTY) == R"({"parent":{"child3":-3}})");
        CHECK(false == root.dirty());
        CHECK(root.persistDirty());
        CHECK(toJson(root, Property::JSON_PERSIST) == R"({"parent":{"child2":-2,"child3":-3}})");
        CHECK(false == root.dirty());
        CHECK(false == root.persistDirty());
    }
}
