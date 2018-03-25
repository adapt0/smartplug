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

Import("env")
# print env.Dump()

# paths
projectbuild_dir = os.path.join(env['PROJECTBUILD_DIR'], 'tests')
projectlib_dir = os.path.join(env['PROJECT_DIR'], 'lib')
doctest_path = os.path.join(projectlib_dir, 'doctest-1.2.7')


# create a separate test environment, targeting the native platform (default)
test_env = Environment(
    CPPDEFINES=[
        'UNIT_TEST',
        'ICACHE_RODATA_ATTR=""',
    ],
    CPPPATH=[
        # env['CPPPATH'],
        doctest_path,
    ],
    CXXFLAGS=env['CXXFLAGS'],
    PROJECT_DIR=env['PROJECT_DIR'],
    PROJECTSRC_DIR=env['PROJECTSRC_DIR'],
    PROJECTTEST_DIR=env['PROJECTTEST_DIR'],
)

test_env.Append(
    # ignore libc clobbering our system includes
    CPPPATH = [p for p in env['CPPPATH'] if not '/libc/' in p],
    CCFLAGS = ['-g', '-ggdb'],
)


def search_cpppaths(source):
    """search CPPPATH for a source file"""
    for cpppath in env['CPPPATH']:
        p = os.path.join(cpppath, source[0])
        if os.path.exists(p):
            return test_env.Object(p, **source[1])
    return source_file


# minimal Arduino sources
arduino_sources = [
    ('core_esp8266_noniso.c', { "CCFLAGS": ['-Wno-absolute-value'] }),
    ('pgmspace.cpp', {}),
    ('WString.cpp', { "CCFLAGS": ['-include', 'cmath' ] }),
]
arduino_lib = test_env.StaticLibrary(os.path.join(projectbuild_dir, 'arduino'), [
    map(search_cpppaths, arduino_sources)
])


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
