# Etekcity SmartPlug firmware + upgrading tool

Alternative firmware for Etekcity's "Voltson Wi-Fi Smart Plug Mini Outlet" (ESW01-USA).

_WARNING: This is an experimental work in progress_

![Pair of Etekcity plugs](doc/etekcity.jpg)

## Projects

### firmware

Alternative firmware

* Web interface
* Powered by [PlatformIO](https://platformio.org/)


### vesync-hijack

Used for loading custom firmware onto Etekcity outlets.

* [vesync-hijack/](vesync-hijack/bootstrap-firmware/)
* [vesync-hijack/bootstrap-firmware/](vesync-hijack/bootstrap-firmware/)

This is able to deploy ESP8266 custom firmware (such as [Tasmota](https://tasmota.github.io/)) provided it's < ~1MB (see #25 for details).

## Documentation

* [internals, taking apart, connecting, backup](doc/internals.md)
* [remote upgrading](doc/upgrading.md)
