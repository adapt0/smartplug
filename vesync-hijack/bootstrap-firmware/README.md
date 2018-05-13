# bootstrap-firmware

ESP8266 SDK compatible firmware for deploying an alternative boot loader + firmware.

## Artifacts

* `firmware.elf` - Our general bootstrap firmware
* `user1.elf` - Re-linked firmware targeting the first OTA partition
* `user1.bin` - ESP8266 OTA SDK binary containing `user1.elf` 
* `user2.elf` - Re-linked firmware targeting the second OTA partition
* `user2.bin` - ESP8266 OTA SDK binary containing `user2.elf` 

`user1.bin` & `user2.bin` are our final artifacts used for device upgrades.

## Generation

[user_bins.py](scripts/user_bins.py) is used to add additional compilation targets to generate our upgrade artifacts. This script is invoked as part of the normal platformio compilation process.

## Links

These links greatly helped with deciphering of the partition layout:

* [Decompiling the ESP8266 boot loader V1.3(B3)](https://richard.burtons.org/2015/05/17/decompiling-the-esp8266-boot-loader-v1-3b3/)
* [gen_appbin.py](https://github.com/espressif/ESP8266_RTOS_SDK/blob/master/tools/gen_appbin.py) (from the ESP8266 RTOS SDK)
* [Memory map](https://github.com/esp8266/esp8266-wiki/wiki/Memory-Map)

