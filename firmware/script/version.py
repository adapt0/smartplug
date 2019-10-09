"""
Generates a version source file dependency

Inspired by:
 https://stackoverflow.com/questions/4942452/how-can-i-add-the-build-version-to-a-scons-build

```
[env:esp12e]
platform = espressif8266
board = esp12e
framework = arduino
extra_scripts =
    scripts/version.py
```

"""

import json
import os
import subprocess
from string import Template

Import("env")
# print env.Dump()


def version_generate_contents():
    """
    Generate version source file
    """
    try:
        # read version.json
        version = json.load(
            open(os.path.join(env["PROJECT_DIR"], "version.json"), 'r')
        )

        # grab git rev
        # https://stackoverflow.com/questions/5143795/how-can-i-check-in-a-bash-script-if-my-local-git-repository-has-changes
        version["gitModified"] = "true" if subprocess.call(["git", "diff-index", "--quiet", "HEAD"], universal_newlines=True) else "false"
        version["gitModifiedStar"] = "*" if "true" == version["gitModified"] else ""
        version["gitRev"] = subprocess.check_output(["git", "rev-parse", "HEAD"], universal_newlines=True).strip()
        version["gitRevShort"] = subprocess.check_output(["git", "rev-parse", "--short", "HEAD"], universal_newlines=True).strip()

        # fill in strings
        version["string"] = Template(
            "v${major}.${minor}.${patch}"
        ).substitute(version)
        version["stringFull"] = Template(
            "v${major}.${minor}.${patch} [${gitRevShort}${gitModifiedStar}]"
        ).substitute(version)
        print(version["stringFull"])

        # populate source
        return Template("""/*
* This file is automatically generated by the build process DO NOT EDIT!
*/

namespace version {
    const int MAJOR = ${major};
    const int MINOR = ${minor};
    const int PATCH = ${patch};

    const char* GIT_REV = "${gitRev}";

    const char* STRING = "${string}";
    const char* STRING_FULL = "${stringFull}";
}
""").substitute(version)

    except IOError as e:
        return "#error " + str(e)


def generate_version_for(target, version_contents):
    """
    Generate version dependency for target
    """

    def write_action(target, source, env):
        """
        Write version contents to source file
        We leave scons to figure out if contents have change
        """
        fd = open(target[0].path, 'w')
        fd.write(version_contents)
        fd.close()
        return 0

    build_version = target.env.Command(
        os.path.join(os.path.dirname(target.get_path()), "src", "__generated_version__.cpp"),
        [],
        write_action
    )
    version_obj = target.env.Object(build_version[0])

    # http://scons.org/doc/production/HTML/scons-user.html#idm139933296986288
    target.env.AlwaysBuild(build_version)
    target.env.Append(LINKFLAGS=version_obj)
    # Requires(target, version_obj)
    target.env.Depends(target, version_obj)


# generate version contents
version_contents = version_generate_contents()

# main elf executable target
elf_target = env.File("$BUILD_DIR/${PROGNAME}.elf")
generate_version_for(elf_target, version_contents)

# tests build target -> tests executable target
tests_build_target = env.Entry("buildtests")
if len(tests_build_target.sources):
    tests_target = tests_build_target.sources[0]
    generate_version_for(tests_target, version_contents)
