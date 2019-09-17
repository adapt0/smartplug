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


# retrieve path to framework-arduinoespressif8266 test stubs
# we use this to pick up their pgmspace.h stub
def get_framework_stubs_path():
    for p in projenv['CPPPATH']:
        if p.endswith('/framework-arduinoespressif8266/tools/sdk/include'):
            return os.path.abspath(os.path.join(p, '../../../tests/host'))
    return None


# create a separate test environment, targeting the native platform (default)
test_env = Environment(
    CPPDEFINES=[
        'UNIT_TEST',
        'ARDUINOJSON_ENABLE_ARDUINO_STREAM',
        'ARDUINOJSON_ENABLE_ARDUINO_STRING',
        'ICACHE_RODATA_ATTR=""',
        # missing lwip defines
        'LWIP_IPV6=0',
        'TCP_MSS=536',
        'LWIP_FEATURES=0',
    ],
    CPPPATH=[
        # projenv['CPPPATH'],
        doctest_path,
        os.path.join(projenv['PROJECTTEST_DIR'], 'stub'),
        get_framework_stubs_path(),
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
    return test_env.Object(source[0], **source[1])


# minimal Arduino sources
arduino_sources_opts = { "CCFLAGS": ['-include', 'cmath' ] }
arduino_sources = [
    ('IPAddress.cpp', arduino_sources_opts),
    ('Print.cpp', arduino_sources_opts),
    ('Stream.cpp', arduino_sources_opts),
    ('StreamString.cpp', arduino_sources_opts),
    ('WString.cpp', arduino_sources_opts),
]
arduino_lib = test_env.StaticLibrary(os.path.join(projectbuild_dir, 'arduino'), list(
    map(search_cpppaths, arduino_sources)
))


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
Default(tests)
