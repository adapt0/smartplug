/////////////////////////////////////////////////////////////////////////////
/** @file
Firmware upgrade management task

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED__TASK_UPGRADE
#define INCLUDED__TASK_UPGRADE

//- includes
#include "task_base.h"

//- forwards
struct ip_addr;

/////////////////////////////////////////////////////////////////////////////
/// upgrade handling task
class TaskUpgrade : public TaskBase {
public:
    enum {
        HTTP_PORT   = 17273,    ///< connect to web server on this TCP port
        MCAST_PORT  = 7001,     ///< multicast announcements UDP port
    };

    void operator()() override;

private:
    uint32_t smartConfig_();
    bool startUpgrade_(const ip_addr* addr);
    bool startUpgradeDance_(const ip_addr* addr);
    size_t retrieveWriteFile_(const ip_addr* addr, const char* filename, unsigned int flashDest, unsigned int flashSize);
};

#endif // INCLUDED__TASK_UPGRADE
