/////////////////////////////////////////////////////////////////////////////
/** @file
Connection task

\copyright Copyright (c) 2019 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED__TASK_CONNECT
#define INCLUDED__TASK_CONNECT

//- includes
#include "task_base.h"
#include <esp_common.h>
#include <freertos/queue.h>

//- forwards
class TaskUpgrade;

/////////////////////////////////////////////////////////////////////////////
/// station connection task
class TaskConnect : public TaskBase {
public:
    TaskConnect(xQueueHandle queue, const char* wifiID, const char* wifiPassword, uint32_t serverIP);

    void operator()();

private:
    bool connectTo_();

    station_config      config_;
    xQueueHandle        queue_;
    uint32_t            serverIP_{0};
};

#endif // INCLUDED__TASK_CONNECT
