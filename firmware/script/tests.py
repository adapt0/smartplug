"""
Build + run DocTest powered tests as part of an environment build

See: https://github.com/onqtam/doctest

```
[env:esp12e]
platform = espressif8266
board = esp12e
framework = arduino
extra_scripts =
    scripts/tests.py
```

Based on:
  https://raw.githubusercontent.com/neuhalje/arduino-aquarium-lights/master/test/test_runner.py
  https://github.com/platformio/platformio-core/issues/519


Wrap src/ code to skip in UNIT_TEST guards:

```
#ifndef UNIT_TEST

// this code won't be compiled

#endif // UNIT_TEST
```

"""

import os
import subprocess

Import("env", "projenv")

# print env.Dump()
# print projenv.Dump()

# paths
projectbuild_dir = os.path.join(projenv['PROJECTBUILD_DIR'], 'tests')
projectlib_dir = os.path.join(projenv['PROJECT_DIR'], 'lib')
doctest_path = os.path.join(projectlib_dir, 'doctest-1.2.7')


# create a separate test environment, targeting the native platform (default)
test_env = Environment(
    CPPDEFINES=[
        'UNIT_TEST',
        'ARDUINOJSON_ENABLE_ARDUINO_STREAM',
        'ARDUINOJSON_ENABLE_ARDUINO_STRING',
        'ICACHE_RODATA_ATTR=""',
    ],
    CPPPATH=[
        # projenv['CPPPATH'],
        doctest_path,
    ],
    CXXFLAGS=projenv['CXXFLAGS'],
    PROJECT_DIR=projenv['PROJECT_DIR'],
    PROJECTSRC_DIR=projenv['PROJECTSRC_DIR'],
    PROJECTTEST_DIR=projenv['PROJECTTEST_DIR'],
)

test_env.Append(
    # ignore libc clobbering our system includes
    CPPPATH = [p for p in projenv['CPPPATH'] if not '/libc/' in p],
    CCFLAGS = ['-g', '-ggdb'],
)

#todo: check if we're using clang
test_env.Append(
    CXXFLAGS  = ['-fsanitize=address', '-fno-omit-frame-pointer'],
    LINKFLAGS = ['-fsanitize=address'],
)

# print test_env.Dump()


def search_cpppaths(source):
    """search CPPPATH for a source file"""
    for cpppath in projenv['CPPPATH']:
        p = os.path.join(cpppath, source[0])
        if os.path.exists(p):
            return test_env.Object(p, **source[1])
    return source


# minimal Arduino sources
arduino_sources_opts = { "CCFLAGS": ['-include', 'cmath' ] }
arduino_sources = [
    ('core_esp8266_noniso.c', { "CCFLAGS": ['-Wno-absolute-value'] }),
    ('IPAddress.cpp', arduino_sources_opts),
    ('Print.cpp', arduino_sources_opts),
    ('pgmspace.cpp', {}),
    ('WString.cpp', arduino_sources_opts),
]
arduino_lib = test_env.StaticLibrary(os.path.join(projectbuild_dir, 'arduino'), [
    map(search_cpppaths, arduino_sources)
])

#############################################################################
# Our own custom check to see if there's a type conflict between the esp & local system headers
# https://scons.org/doc/1.3.1/HTML/scons-user/x4113.html
#
# I've seen this on Ubuntu 18.04.1 with gcc-7
# While clang 4.2.1 on my Mac is OK
#
# Rather than play preprocessor games, we disable our default test execution
# when type conflicts are a problem
#
# /usr/include/x86_64-linux-gnu/sys/types.h:181:1: note: previous declaration as 'typedef long unsigned int u_int64_t'
# __u_intN_t (64, __DI__);
# ~/.platformio/packages/framework-arduinoespressif8266/tools/sdk/include/c_types.h:36:29: error: conflicting declaration 'typedef long long unsigned int u_int64_t'
#  typedef unsigned long long  u_int64_t;
def CheckTypesOk(context):
    test_type_conflict_source = """
    #include <c_types.h>
    #include <sys/types.h>

    int main(int, char**) {
        return 0;
    }
    """

    context.Message('Checking for header type conflict...')
    result = context.TryLink(test_type_conflict_source, '.cpp')
    context.Result(result)
    return result
conf_test = Configure(test_env, custom_tests = {'CheckTypesOk' : CheckTypesOk})
# conf_test is checked below

#############################################################################
# Create a builder for tests
def builder_unit_test(target, source, env):
    return subprocess.call([ source[0].abspath ])
bld = Builder(action=builder_unit_test)
test_env.Append(BUILDERS={'Test': bld})

# tests
program = test_env.Program(os.path.join(projectbuild_dir, 'tests'), [
    Glob(os.path.join(test_env['PROJECTTEST_DIR'], '*.cpp')),
    Glob(os.path.join(test_env['PROJECTSRC_DIR'], '*.cpp')),
], LIBS = [arduino_lib])

tests = [test_env.Test("buildtests", program)]
if conf_test.CheckTypesOk():
    Default(tests)
