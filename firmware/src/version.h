/////////////////////////////////////////////////////////////////////////////
/** @file
Version information

Data is generated during the build by scripts/version.py

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED__VERSION
#define INCLUDED__VERSION

/////////////////////////////////////////////////////////////////////////////
/// expose constants from generated version file (see scripts/version.py)
namespace version {
    extern const int MAJOR;
    extern const int MINOR;
    extern const int PATCH;

    extern const char* GIT_REV;

    extern const char* STRING;
    extern const char* STRING_FULL;
}

#endif // INCLUDED__VERSION
