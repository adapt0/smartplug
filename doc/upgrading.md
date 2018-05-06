# Upgrading

Now say you're the kind of person who doesn't want to dismantle their outlet, solder on some wires, and establish a physical serial connection. Wires? What is this? The 90's?

We know that this plug has network connectivity. We also know that it's capable of accepting new firmware. So ... pull up a seat!

Let's review the steps required to set up a plug:
* Power up plug
* Put plug into some kind of discovery mode
* Use a program to discover + configure plug
* Plug now connects + controlled by the cloud

## Discovery

There's actually two discovery modes these plugs can be in:

1. First is what is known as "Smart Config" or "AirKiss" mode. This mode is accessed by holding down the plug's button until the LED glows solid blue.
2. Second mode is "APN" mode, with the plug advertising itself as an access point. Holding down the outlet button for ten seconds until the light starts to slowly blink blue will enter this mode.

AirKiss seems to be developed by X and is scary genius. It works by advertising a WiFi network's SSID & password through packet lengths! Devices passively channel hop sniffing for "guide codes". On seeing these guide codes, the device will lock onto the respective channel, then sniff for the WiFi settings.

* [AirKiss links]

Once the WiFi settings are known, the plug will connect to the advertised network, and begin announcing itself via UDP broadcasts on port 18266.

```
0x1b + MAC + IP
```

## Begin Configuration

On receiving a UDP announcement we know where to initiate a TCP connection on port 41234. This TCP connection only seems to respond to the `/beginConfigRequest` command, which triggers the device to establish a WebSocket connection to the indicated web server on port 17273 with the path `/gnws`.

```
// establish TCP connection

// send /beginConfigRequest (notice that the length byte prefix)
const msg = JSON.stringify{
    'uri' : '/beginConfigRequest',
    'wifiID' : this.apSsid_,
    'wifiPassword' : this.apPass_,
    'account' : 'dummyAccount',
    'key' : 123,
    'serverIP' : this.ipAddress_,
});
client.write( Buffer.concat([
    Buffer.from([ msg.length ]),
    Buffer.from(msg),
]), resolve );
```

## WebSocket

On establishing a WebSocket connection the device will attempt to `/login`. We can provide a `/loginReply` to satisfy immediate needs.

```
const d = new Date();
ws.send(JSON.stringify({
    uri: '/loginReply',
    error: 0,
    wd: 3,
    year: d.getFullYear(),
    month: 1 + d.getMonth(),
    day: d.getDate(),
    ms: d.getTime(),
    hh: 0,
    hl: 0,
    lh: 0,
    ll: 0
}));
```

We can then finally initiate an `/upgrade` request with a base URL the device can use to retrieve new firmware.

```
ws.send(JSON.stringify({
    uri: '/upgrade',
    url: `http://${this.ipAddress_}:${this.httpPort_}`,
    newVersion: '2.00',
}));
```

Device will then request either `/upgrade/user1.bin` or `/upgrade/user2.bin` depending on which partition its current firmware is running out of. These images do differ slightly, as internal address offsets need to account for their different locations within flash.

## Hostile takeover

Taking this one step further. We can create some initial bootstrap images. This ensures that we are running from the second firmware partition. Which allows us to rewrite the first image, along with the boot loader, to completely use our own environment.

Final flow:

1. Periodically send out AirKiss packets to have smart plugs join our WiFi
2. Listens for UDP announcements from joined plugs
3. Initiates a TCP connection with joined plugs, instructing them to use our web server
4. Accepts WebSocket requests from newly configured plugs, initiates a firmware upgrade
5. Serves up bootstrap firmware images (user1.bin/user2.bin)
6. Serves up our final complete image (firmware.bin)
