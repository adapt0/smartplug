/////////////////////////////////////////////////////////////////////////////
/** @file
Connection task

\copyright Copyright (c) 2019 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////

//- includes
#include "task_connect.h"

/////////////////////////////////////////////////////////////////////////////
/// station connection task
TaskConnect::TaskConnect(xQueueHandle queue, const char* wifiID, const char* wifiPassword, uint32_t serverIP)
: queue_{queue}
, serverIP_{serverIP}
{
    memset(&config_, 0, sizeof(config_));
    strncpy((char*)config_.ssid, wifiID, sizeof(config_.ssid));
    strncpy((char*)config_.password, wifiPassword, sizeof(config_.password));
}

/////////////////////////////////////////////////////////////////////////
/// establish connection
void TaskConnect::operator()() {
    if (connectTo_()) {
        xQueueSend(queue_, &serverIP_, 0);
    } else {
        xQueueSend(queue_, nullptr, 0); // failed, we need to reinitialize after connectTo_
    }
}

/////////////////////////////////////////////////////////////////////////
/// begin establishing connection
bool TaskConnect::connectTo_() {
    smartconfig_stop();

    printf("Attempting to connect to AP\n");

    //
    wifi_station_set_config_current(&config_);

    wifi_station_disconnect();
    if (!wifi_set_opmode_current(STATION_MODE)) printf("STATION_MODE failed\n");

    if (!wifi_station_connect()) {
        printf("wifi_station_connect failed\n");
        return false;
    }

    for (int attempt = 0; attempt < 60; ++attempt) {
        vTaskDelay(1000 / portTICK_RATE_MS);

        const auto status = wifi_station_get_connect_status();
        printf("status = %d\n", status);
        if (STATION_GOT_IP == status) {
            printf("Connected!\n");
            return true;
        }

        if (STATION_CONNECT_FAIL == status || STATION_WRONG_PASSWORD == status) break;
    }

    printf("Failed to connect\n");
    return false;
}
