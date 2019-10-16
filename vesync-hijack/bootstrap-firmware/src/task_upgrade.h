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
#include <freertos/queue.h>

//- forwards
struct cJSON;
struct espconn;
struct ip_addr;

/////////////////////////////////////////////////////////////////////////////
/// upgrade handling task
class TaskUpgrade : public TaskBase {
public:
    enum {
        HTTP_PORT   = 17273,    ///< connect to web server on this TCP port
        MCAST_PORT  = 7001,     ///< multicast announcements UDP port
        TCP_PORT    = 41234,    ///< TCP control port
    };

    void operator()() override;

private:
    /// TCP client data
    struct TcpClient {
        explicit TcpClient(TaskUpgrade* task_) : task{task_} { }

        TaskUpgrade* const  task;           ///< associated parent task
        char                data[256];
        uint8_t             length = 0;
        uint8_t             received = 0;
    };

    static void tcpOnAccept_(void* arg);
    static void tcpOnDisconnect_(void* arg);
    static void tcpOnRecv_(void* arg, char* pusrdata, unsigned short length);
    void tcpOnRecvMessage_(espconn* pesp_conn, cJSON* object);

    uint32_t smartConfig_();
    bool startUpgrade_(const ip_addr* addr);
    bool startUpgradeDance_(const ip_addr* addr);
    size_t retrieveWriteFile_(const ip_addr* addr, const char* filename, unsigned int flashDest, unsigned int flashSize);

    static TaskUpgrade* instance_;
    xQueueHandle    config_queue_{nullptr}; ///< queue for smartConfig_/connectTo_ notifications
};

#endif // INCLUDED__TASK_UPGRADE
