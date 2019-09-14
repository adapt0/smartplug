/////////////////////////////////////////////////////////////////////////////
/** @file
doctest extensions

\copyright Copyright (c) 2019 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED__TEST__DOCTEST_EXT
#define INCLUDED__TEST__DOCTEST_EXT

//- includes
#include <doctest/doctest.h>
#include <IPAddress.h>

std::ostream& operator<<(std::ostream& outs, const IPAddress& ip);
std::ostream& operator<<(std::ostream& outs, const String& str);

#endif // INCLUDED__TEST__DOCTEST_EXT
