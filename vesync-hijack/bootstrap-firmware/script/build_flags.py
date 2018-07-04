#!/usr/bin/env python
# Copyright (c) 2018 Chris Byrne. All rights reserved.
# Licensed under the MIT License. Refer to LICENSE file in the project root.

Import("env")

# use extra_script to adjust c++ flags:
#  https://github.com/platformio/platformio-core/issues/462
# Bug 61653 - Warning 'literal-suffix' is not suppressed by gcc pragma
#  https://gcc.gnu.org/bugzilla/show_bug.cgi?id=61653

# ip6_addr.h:263:85: warning: invalid suffix on literal; C++11 requires a space between literal and identifier [-Wliteral-suffix]
env.Append(CXXFLAGS=["-Wno-literal-suffix"])
