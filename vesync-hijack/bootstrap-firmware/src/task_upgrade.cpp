/////////////////////////////////////////////////////////////////////////////
/** @file
Firmware upgrade management task

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////

//- includes
#include "task_upgrade.h"
#include "flash_writer.h"
#include "http_connection.h"
#include "net_conn.h"
#include <esp_common.h>
#include <upgrade.h>

//- externals
extern unsigned int _irom0_text_start;  ///< irom0 start address

/////////////////////////////////////////////////////////////////////////////
/// Upgrade task entry point
void TaskUpgrade::operator()() {
    // start network
    startNetwork_();

    // wait for multicast announcement packets
    {
        struct ip_addr mcast_addr;
        IP4_ADDR(&mcast_addr, 234,100,100,100);

        NetConn udp{NETCONN_UDP};
        udp.setRecvTimeout(1000); // ms
        udp.joinLeaveGroup(&mcast_addr, IP_ADDR_ANY, NETCONN_JOIN);
        udp.bind(IP_ADDR_ANY, MCAST_PORT);

        while (true) {
            printf("status: %d\r\n", wifi_station_get_connect_status());

            auto pkt = udp.recv();
            if (!pkt) continue;

// printf("UDP Packet! %d\n", pkt.length);

            // expect packets to be filled with ones
            if (pkt.size() > 0 && '1' == pkt.data()[0]) {
                auto* addr = pkt.fromAddr();
                if (startUpgrade_(addr)) break;
                system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
            }
        }

        printf("Done!");
    }
}

/////////////////////////////////////////////////////////////////////////////
/// start network
void TaskUpgrade::startNetwork_() {
    // connect to AP
    station_config config;
    // if (wifi_station_get_config_default(&config)) {
    if (wifi_station_get_config(&config)) {
printf("SSID: %s\r\n", config.ssid);
        if (!wifi_set_opmode_current(STATION_MODE)) {
            printf("Failed to set STATION mode :(\r\n");
        }

printf("status: %d\r\n", wifi_station_get_connect_status());

        wifi_station_connect();
    }

//TODO: fall back on AP if STA is unavailable

    // wait for an IP
    while (STATION_GOT_IP != wifi_station_get_connect_status()) {
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}



/////////////////////////////////////////////////////////////////////////////
/// attempt to retrieve upgrade
bool TaskUpgrade::startUpgrade_(const ip_addr* addr) {

// determine where our irom lives
const int iromAddr = (int)&_irom0_text_start;
printf("0x%x\r\n", iromAddr);

const int romBase  = 0x40200000;
const int romUser2 = 0x40281000;

// if (iromAddr >= romUser2) {
//     printf("We're running out of user2!\r\n");
//     return false;
// }

if (system_upgrade_userbin_check() == UPGRADE_FW_BIN1) {
    printf("user2.bin\r\n");
} else if (system_upgrade_userbin_check() == UPGRADE_FW_BIN2) {
    printf("user1.bin\r\n");
}

    // we need to be running out of user2
    const bool upgradeUser2 = (iromAddr < romUser2);
    const auto  romDest = (upgradeUser2) ? romUser2     : romBase;
    const auto* romName = (upgradeUser2) ? "/user2.bin" : "/firmware.bin";

    //
    HttpConnection http{addr, TaskUpgrade::HTTP_PORT};
    if (ERR_OK != http.get(romName)) {
        printf("Connection failed\n");
        return false;
    }
printf(">> status = %d\r\n", http.statusCode());

    if (200 != http.statusCode()) {
        printf("Unexpected status code %d\n", http.statusCode());
        return false;
    }

printf(">> contentLength = %d\r\n", http.contentLength());


    system_upgrade_flag_set(UPGRADE_FLAG_START);


    // erase time
    {
        const int sectors = (http.contentLength() + SPI_FLASH_SEC_SIZE - 1) / SPI_FLASH_SEC_SIZE;
        const int sectorOfs = (romDest - romBase) / SPI_FLASH_SEC_SIZE;
printf("erase sectors = %d\r\n", sectors);
printf("sectorOfs = %d\r\n", sectorOfs);

        for (int i = 0; i < sectors; ++i) {
            printf("Erase sector %d\r\n", sectorOfs + i);
            if (SPI_FLASH_RESULT_OK != spi_flash_erase_sector(sectorOfs + i)) {
                printf("erase failed :(\r\n");
                return false;
            }
        }
    }


    // write content
    FlashWriter flashWriter{romDest - romBase};
    while (true) {
        const auto available = http.available();
        if (available <= 0) break;

        //
        const auto res = flashWriter.spiWrite(http.data(), available);
        if (SPI_FLASH_RESULT_OK != res) {
            printf("Flash write failed :(\r\n", res);
            return false;
        }

        //
        if (ERR_OK != http.next()) {
            printf("Failed to read\n");
            return false;
        }
    }

    {
        const auto res = flashWriter.spiWriteFinal();
        if (SPI_FLASH_RESULT_OK != res) {
            printf("Flash write failed :(\r\n", res);
            return false;
        }
    }

    printf("Write complete\r\n");

// // if (upgrade_crc_check(system_get_fw_start_sec(), contentOfs) != true) {
//     // printf("upgrade crc OK !\r\n");
// // } else {
//     // printf("upgrade crc check failed !\r\n");
//     // system_upgrade_flag_set(UPGRADE_FLAG_IDLE);
// // }

    //
    system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
    if (UPGRADE_FLAG_FINISH == system_upgrade_flag_check()) {
//TODO: need to do a different reset for firmware.bin
        // if (upgradeUser2)
        system_upgrade_reboot(); // if needed
    }

    return true;
}
