/////////////////////////////////////////////////////////////////////////////
/** @file
General utility functions

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED__UTILS
#define INCLUDED__UTILS

//- forwards
class IPAddress;

namespace utils {

bool validSubnet(const IPAddress& subnet);

} // namespace utils

#endif // INCLUDED__UTILS
