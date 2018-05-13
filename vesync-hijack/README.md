# vesync-hijack

Used for loading custom firmware onto Etekcity outlets.

* [bootstrap-firmware](bootstrap-firmware/README.md)

## Compiling bootstrap firmware

Compiling the bootstrap firmware (requires PlatformIO):

```
cd vesync-hijack/bootstrap-firmware
pio run
```

`user1.bin` & `user2.bin` artifacts can be found in `.pioenvs/app`

## Hijacking

Running the hijack process:

```
cd vesync-hijack/hijacking-server
npm install
node ./index.js -p <wifi_password>
```
