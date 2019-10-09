/////////////////////////////////////////////////////////////////////////////
/** @file
Firmware upgrade management task

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////

//- includes
#include "task_upgrade.h"
#include "eboot_command.h"
#include "flash_writer.h"
#include "http_connection.h"
#include "net_conn.h"
#include <esp_common.h>
#include <smartconfig.h>
#include <upgrade.h>

//- externals
extern unsigned int _irom0_text_start;  ///< irom0 start address

//- constants
const unsigned int SPI_FLASH_BASE = 0x40200000; ///< start of SPI flash
const int iromAddr = (int)&_irom0_text_start;   ///< address we are running out of

/////////////////////////////////////////////////////////////////////////////
/// Upgrade task entry point
void TaskUpgrade::operator()() {
    while (true) {
        // perform smart config
        const auto ip4 = smartConfig_();
        if (0 == ip4) {
            vTaskDelay(1000 / portTICK_RATE_MS);
            continue;
        }

        //
        ip_addr addr;
        ip4_addr_set_u32(&addr, ip4);
        if (!startUpgrade_(&addr)) {
            vTaskDelay(1000 / portTICK_RATE_MS);            
            continue;
        }
        system_upgrade_flag_set(UPGRADE_FLAG_IDLE);

        // wait a bit
        vTaskDelay(5000 / portTICK_RATE_MS);

        printf("Done!");
        break;
    }
}

/////////////////////////////////////////////////////////////////////////////
/// perform smart config
uint32_t TaskUpgrade::smartConfig_() {
    wifi_station_disconnect();
    if (!wifi_set_opmode_current(STATION_MODE)) {
        printf("Failed to set STATION mode :(\r\n");
    }

    static auto sc_queue = xQueueCreate(1, sizeof(uint32_t));

    // based on examples/smart_config/user/user_main.c
    smartconfig_start([](sc_status status, void* pdata) {
        switch(status) {
        case SC_STATUS_WAIT:
            printf("SC_STATUS_WAIT\n");
            break;
        case SC_STATUS_FIND_CHANNEL:
            printf("SC_STATUS_FIND_CHANNEL\n");
            break;
        case SC_STATUS_GETTING_SSID_PSWD:
        {
            printf("SC_STATUS_GETTING_SSID_PSWD\n");
            const auto* type = (sc_type*)pdata;
            if (*type == SC_TYPE_ESPTOUCH) {
                printf("SC_TYPE:SC_TYPE_ESPTOUCH\n");
            } else {
                printf("SC_TYPE:SC_TYPE_AIRKISS\n");
            }
            break;
        }
        case SC_STATUS_LINK:
        {
            printf("SC_STATUS_LINK\n");
            auto* sta_conf = (station_config*)pdata;
    
            wifi_station_set_config(sta_conf);
            wifi_station_disconnect();
            wifi_station_connect();
            break;
        }
        case SC_STATUS_LINK_OVER:
            printf("SC_STATUS_LINK_OVER\n");
            if (pdata) {
                uint8 phone_ip[4] = {0};
                memcpy(phone_ip, pdata, sizeof(phone_ip));
                printf("Phone ip: %d.%d.%d.%d\n", phone_ip[0], phone_ip[1], phone_ip[2], phone_ip[3]);

                xQueueSend(sc_queue, phone_ip, 0);
            }
            smartconfig_stop();
            break;
        }
    });

    //
    uint32_t ip{0};
    if (pdTRUE == xQueueReceive(sc_queue, &ip, 5 * 60 * 1000 / portTICK_RATE_MS)) {
        printf("xQueueReceive %x\n", ip);
        return ip;
    }

    //
    printf("Smart config timeout. Retrying\n");
    smartconfig_stop();

    return 0;
}

/////////////////////////////////////////////////////////////////////////////
/// begin to retrieve an upgrade, writing to a high memory location
/// then we rely on the eboot to handle moving our data on boot similar to OTA
bool TaskUpgrade::startUpgrade_(const ip_addr* addr) {
    // we start at 2MB in the event we ever use a 1M+1M partitioning scheme, currently we're 512+512
    const unsigned int flashBaseAddr = 0;
    const unsigned int flashDestAddr = 0x200000; // write at 2MB
    const unsigned int MAX_ROM_SIZE = 0x100000; // we don't support ROMs > 1MB

    // 1) retrieve firmware.bin, writing to a high memory location
    const auto contentLength = retrieveWriteFile_(addr, "/firmware.bin", SPI_FLASH_BASE + flashDestAddr, MAX_ROM_SIZE);
    if (contentLength <= 0) return false;

    // 2) sanity check that we're using an eboot loader
    //TODO: check boot loader is going to accept our instructions before we just replace it ...

    // 3) write boot loader instructions to move it
    Eboot::writeCommand(Eboot::Command{
        Eboot::Action::COPY_RAW,
        { flashDestAddr, flashBaseAddr, contentLength }
    });

    // 4) overwrite boot loader
    printf("writing boot loader\r\n");
    {
        uint32_t bootLoaderData[SPI_FLASH_SEC_SIZE / sizeof(uint32_t)];
        static_assert( 4096 == sizeof(bootLoaderData), "Expected boot loader to be 4k" );
        spi_flash_read(flashDestAddr, bootLoaderData, sizeof(bootLoaderData));

        // erase
        if (SPI_FLASH_RESULT_OK != spi_flash_erase_sector(flashBaseAddr)) {
            printf("erase boot loader failed :(\r\n");
            return false;
        }

        // write
        FlashWriter flashWriter{flashBaseAddr};
        auto res = flashWriter.spiWrite(bootLoaderData, sizeof(bootLoaderData));
        if (SPI_FLASH_RESULT_OK != res) {
            printf("boot loader write failed :(\r\n", res);
            return 0;
        }
        res = flashWriter.spiWriteFinal();
        if (SPI_FLASH_RESULT_OK != res) {
            printf("boot loader write failed :(\r\n", res);
            return 0;
        }
    }

    // 5) initiate a reboot
    system_restart();

    return true;
}

/////////////////////////////////////////////////////////////////////////////
/// perform an upgrade dance
/// where we ensure we are running out of the second firmware partition
/// then overwrite the first partition with our own firmware
bool TaskUpgrade::startUpgradeDance_(const ip_addr* addr) {
    const auto romBase  = SPI_FLASH_BASE;
    const auto romUser2 = SPI_FLASH_BASE + 0x81000; // User application, slot 2

    //
    system_upgrade_flag_set(UPGRADE_FLAG_START);

    // we need to be running out of user2
    const bool upgradeUser2 = (iromAddr < romUser2);
    const auto  romDest = (upgradeUser2) ? romUser2     : romBase;
    const auto* romName = (upgradeUser2) ? "/user2.bin" : "/firmware.bin";

    // retrieve + write file
    const auto contentLength = retrieveWriteFile_(addr, romName, romDest, romUser2 - romDest);
    if (contentLength <= 0) return false;

    //TODO: could verify the CRC here
    // we'll leave the boot loader to do it

    //
    system_upgrade_flag_set(UPGRADE_FLAG_FINISH);
    if (UPGRADE_FLAG_FINISH == system_upgrade_flag_check()) {
        system_upgrade_reboot(); // if needed
    }

    return true;
}

/////////////////////////////////////////////////////////////////////////////
/// retrieve a file over HTTP and write it to the specified flash location
/// where we ensure we are running out of the second firmware partition
/// then overwrite the first partition with our own firmware
/// @returns size of received firmware
size_t TaskUpgrade::retrieveWriteFile_(const ip_addr* addr, const char* filename, unsigned int flashDest, unsigned int flashSize) {
    printf("Requesting %s\n", filename);

    //
    HttpConnection http{addr, TaskUpgrade::HTTP_PORT};
    if (ERR_OK != http.get(filename)) {
        printf("Connection failed\n");
        return 0;
    }
    // printf(">> status = %d\r\n", http.statusCode());

    if (200 != http.statusCode()) {
        printf("Unexpected status code %d\n", http.statusCode());
        return 0;
    }

    // sanity check that the image is going to fit
    const auto contentLength = http.contentLength();
    // printf(">> contentLength = %d\r\n", http.contentLength());
    if (contentLength > flashSize) {
        printf("Firmware image is too large!! (%d bytes)\n", contentLength);
        return 0;
    }

    // erase time
    const unsigned int sectorBase = (flashDest - SPI_FLASH_BASE) / SPI_FLASH_SEC_SIZE;
    {
        const auto sectorCount = (contentLength + SPI_FLASH_SEC_SIZE - 1) / SPI_FLASH_SEC_SIZE;
        const auto sectorLast = sectorBase + sectorCount;
        for (unsigned int sector = sectorBase; sector < sectorLast; ++sector) {
            printf("Erase sector %d\r\n", sector);
            if (SPI_FLASH_RESULT_OK != spi_flash_erase_sector(sector)) {
                printf("erase failed :(\r\n");
                return 0;
            }
        }
    }

    // write content
    FlashWriter flashWriter{sectorBase * SPI_FLASH_SEC_SIZE};
    while (true) {
        const auto available = http.available();
        if (available <= 0) break;

        //
        const auto res = flashWriter.spiWrite(http.data(), available);
        if (SPI_FLASH_RESULT_OK != res) {
            printf("Flash write failed :(\r\n", res);
            return 0;
        }

        //
        if (ERR_OK != http.next()) {
            printf("Failed to read\n");
            return 0;
        }
    }

    {
        const auto res = flashWriter.spiWriteFinal();
        if (SPI_FLASH_RESULT_OK != res) {
            printf("Flash write failed :(\r\n", res);
            return 0;
        }
    }

    printf("Write complete\r\n");
    return contentLength;
}
