# Etekcity SmartPlug firmware + upgrading tool

_WARNING: This is an abandoned experiment. Feel free to use for research purposes_

Alternative firmware for Etekcity's "Voltson Wi-Fi Smart Plug Mini Outlet" (ESW01-USA).

![Pair of Etekcity plugs](doc/etekcity.jpg)

## Projects

### firmware

Alternative firmware

* Web interface
* Powered by [PlatformIO](https://platformio.org/)

_This fails to compile as PlatformIO no longer includes the test mocks in framework-arduinoespressif8266. If you want to compile this, comment out tests.py in platformio.ini._

### vesync-hijack

Used for loading custom firmware onto Etekcity outlets.

* [vesync-hijack/](vesync-hijack/bootstrap-firmware/)
* [vesync-hijack/bootstrap-firmware/](vesync-hijack/bootstrap-firmware/)

This is able to deploy ESP8266 custom firmware (such as [Tasmota](https://tasmota.github.io/)) provided it's < ~1MB (see #25 for details).

## Documentation

* [internals, taking apart, connecting, backup](doc/internals.md)
* [remote upgrading](doc/upgrading.md)
