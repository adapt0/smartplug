/////////////////////////////////////////////////////////////////////////////
/** @file
Etekcity/Vesync smart plug firmware hijacking server

# Periodically sends out AirKiss packets to have smart plugs join our WiFi
# Listens for UDP announcements from joined plugs
# Initiates a TCP connection with joined plugs, instructs them to use our web server
# Accepts WebSocket requests from newly configured plugs, initiates a firmware upgrade
# Serves up bootstrap firmware images (user1.bin/user2.bin)
# Serves up our final complete image (firmware.bin)

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////

const ChildProcess = require('child_process');
const commander = require('commander');
const crypto = require('crypto');
const dgram = require('dgram'); 
const fs = require('fs');
const http = require('http');
const morgan = require('morgan');
const net = require('net');
const os = require('os');
const path = require('path');
const WebSocket = require('ws');

/////////////////////////////////////////////////////////////////////////////
/// CRC8 calculation using 0x8C polynomial (reversed 1-wire?)
/// https://github.com/EspressifApp/EsptouchForAndroid/blob/master/src/com/espressif/iot/esptouch/util/CRC8.java
const crc8Esptouch = (() => {
    const polyNom = 0x8c;
    const initial = 0x00;

    const table = new Array(256);
    for (let i = 0; i < table.length; ++i) {
        let remainder = i;
        for (let bit = 0; bit < 8; ++bit) {
            if (remainder & 0x1) {
                remainder = (remainder >>> 1) ^ polyNom;
            } else {
                remainder >>>= 1;
            }
        }
        table[i] = (remainder & 0xffff);
    }

    function crc8(bytes) {
        // special case for string
        if ('string' === typeof(bytes)) return crc8(Buffer.from(bytes));

        let value = initial;
        for (let i = 0; i < bytes.length; ++i) {
            const b = bytes[i] ^ value;
            value = (table[b & 0xff] ^ ((value << 8) & 0xffff));
        }

        return value & 0xff;
    }

    return crc8;
})();

/////////////////////////////////////////////////////////////////////////////
/// elapsed seconds (monotomic)
const elapsedSeconds = () => {
    const [seconds, nanos] = process.hrtime();
    return seconds + nanos / 1e9;
};

/////////////////////////////////////////////////////////////////////////////
/// our hijack servers
class VesyncHijack {
    constructor() {
        // grab our version from package.json
        const packageJson = require('./package.json');
        this.version_ = packageJson.version;

        //
        this.deviceConfigPort_ = 41234;
        this.listenPort_ = 18266;
        this.httpPort_ = 17273;

        // airkiss address + port
        this.mutlicastAddr_ = '234.100.100.100';
        this.multicastPort_ = 7001;

        this.ssidHidden_ = true;
    }

    /////////////////////////////////////////////////////////////////////////
    /// spin up servers
    async begin() {
        // parse command line
        commander
            .version(this.version_)
            .option('-i, --ip <value>')
            .option('-s, --ssid <value>')
            .option('-b, --bssid <value>')
            .option('-p, --password <value>')
            .option('-d, --device <value>')
            .parse(process.argv)
        ;

        if (commander.device) commander.bssid = ''; // not needed

        // console.log('Retrieving WiFi settings');
        await this.getNetworkInfo_(commander.ssid, commander.bssid, commander.password, commander.ip);
        console.log(`Using SSID "${this.apSsid_}" (BSSID: ${this.apBssidStr_}, Local IP: ${this.ipAddress_})`);

        const promises = [];

        // start our web server
        promises.push(this.beginHttpServer_());

        if (commander.device) {
            // connect directly to specified device
            promises.push((async () => {
                console.log(`Attempting to connect to device ${commander.device}`);
                await this.connectToDevice_(commander.device);
            })());

        } else {
            // listen for device UDP announcements, direct them to our web server
            promises.push(this.beginUdp_());

            // kick off Smart Config (beginUdp_ must be called first)
            console.log('Sending Smart Config...');
            promises.push(this.beginAirKiss_());
        }

        await Promise.all(promises);
    }

    /////////////////////////////////////////////////////////////////////////
    /// get current WiFi config
    /// TODO: should node module, as we need the BSSID which other modules don't provide
    getWifiInfo_() {
        if (process.platform === 'darwin') {
            // OS X
            const airport = String(ChildProcess.execFileSync('system_profiler', ['SPAirPortDataType', '-detailLevel', 'basic']));
            if (!airport) throw new Error('Failed to retrieve WiFi settings');
            // console.log(airport);

            const res = {
                interface: /Interfaces:\s+([^\s:]+)/,
                status:    /Status:\s+([\S]+)/,
                phyMode:   /PHY Mode:\s+([\S]+)/,
                ssid:      /Current Network Information:\s+([^\s]+):/,
                bssid:     /BSSID:\s+([\S]+)/,
            };

            let success = true;
            Object.keys(res).forEach((k) => {
                const m = airport.match(res[k]);
                if (!m) success = false;
                res[k] = (m) ? m[1] : null;
            });
            res.success = success;
            return res;
        } else {
            throw new Error(`Unsupported platform ${process.platform}. Please use --ssid and --bssid to provide WiFi details`);
        }
    }

    /////////////////////////////////////////////////////////////////////////
    /// retrieve network information
    async getNetworkInfo_(apSsid, apBssid, apPassword, ipAddress) {
        // find first non-internal IPv4 interface 
        const ourIp = (() => {
            for (const [name, addresses] of Object.entries(os.networkInterfaces())) {
                const f = addresses.find((addr) => (false === addr.internal && 'IPv4' === addr.family));
                if (f) {
                    if (ipAddress) {
                        if (name === ipAddress || ipAddress === f.address) return f.address;
                    } else {
                        console.log(`Using ${name} ${f.address}`);
                        return f.address;
                    }
                }
            }
            return null;
        })();
        if (!ourIp) throw new Error('Failed to find our IP address');

        // AP password
        if (apPassword == null) throw new Error('Need to specify your WiFi password (-p)');
        this.apPass_ = apPassword;

        //
        if (null != apSsid) {
            // use provided wireless details
            this.apSsid_ = apSsid;
            this.apBssidStr_ = apBssid;
        } else {
            // find local wifi device
            console.log('Looking for local WiFi interface (use --ssid & --bssid to override)');
            const res = this.getWifiInfo_();
            if (!res || !res.success) {
                throw new Error('Failed to find WiFi interface');
            }
            // console.log(res, state);
            if ('connected' !== (res.status || '').toLowerCase()) {
                throw new Error(`WiFi '${res.interface} is disconnected?`);
            }

            if (apSsid && apSsid !== res.ssid) {
                throw new Error(`Specified WiFi SSID "${apSsid}" doesn't match connected "${res.ssid}"?`);
            }

            // fill in SSID
            this.apSsid_ = res.ssid;
            this.apBssidStr_ = res.bssid;
        }
        if (!this.apBssidStr_) this.apBssidStr_ = '00:00:00:00:00:00';
        this.apBssid_ = this.apBssidStr_.split(':').map(b => parseInt(b, 16));

        //
        this.ipAddress_ = ourIp;
    }

    /////////////////////////////////////////////////////////////////////////
    /// begin listening for device UDP announcements
    async beginUdp_() {
        // UDP socket for Smart Config + new device announcements
        this.udpSocket_ = dgram.createSocket('udp4');
        await new Promise((resolve) => {
            this.udpSocket_.bind(this.listenPort_, this.ipAddress_, resolve);
            console.log(`Listening for devices on UDP port ${this.listenPort_}`);
        });
        this.udpSocket_.setMulticastInterface(this.ipAddress_);
        this.udpSocket_.setBroadcast(true);
        this.udpSocket_.setMulticastTTL(1);

        this.udpLastDevice_ = null;
        this.udpLastDeviceTime_ = 0;

        this.udpSocket_.on('message', async (data, from) => {
            // console.log(data);
            // reply: 0x1b + MAC + IP

            const ipDevice = from.address;

            // ignore multiple packets from the same device in a short period of time
            if (ipDevice === this.udpLastDevice_ && ((elapsedSeconds() - this.udpLastDeviceTime_) < 3)) {
                return;
            }
            this.udpLastDevice_ = ipDevice;
            this.udpLastDeviceTime_ = elapsedSeconds();

            await this.connectToDevice_(ipDevice);
        });
    }

    /////////////////////////////////////////////////////////////////////////
    /// generate packet lengths for Smart Config protocol
    airKissPacketLengths_() {
        // https://github.com/EspressifApp/EsptouchForAndroid/blob/master/src/com/espressif/iot/esptouch/protocol/DatumCode.java
        // define by the Esptouch protocol, all of the datum code should add 1 at last to prevent 0
        const extraLength = 40;

        // gather data bytes to encode
        const ipAddressBytes = this.ipAddress_.split('.').map(b => parseInt(b));
        let dataBytes = Buffer.concat([
            Buffer.from([
                0,                              // total length (filled in below)
                this.apPass_.length,            // AP password length
                crc8Esptouch(this.apSsid_),     // SSID CRC8
                crc8Esptouch(this.apBssid_),    // BSSID CRC8
                0                               // totalXor (filled in below)
            ]),
            Buffer.from(ipAddressBytes),        // ipAddress
            Buffer.from(this.apPass_),          // AP Password
            Buffer.from(this.apSsid_),          // AP SSID (kept if hidden)
        ]);

        // total data byte length
        const totalLength = dataBytes.length;
        dataBytes[0] = totalLength;

        // XOR data bytes
        const totalXor = dataBytes.reduce((b, out) => out ^ b, 0);
        dataBytes[4] = totalXor;

        // if SSID isn't hidden, then remove from payload
        // note that we needed it in the payload for totalLength + totalXor calculations
        if (!this.ssidHidden_) dataBytes = dataBytes.slice(0, dataBytes.length - this.apSsid_.length);

        const packetLengths = [];
        for (let seq = 0; seq < dataBytes.length; ++seq) {
            const b = dataBytes[seq];
            const sum = crc8Esptouch([b, seq]);

            // https://github.com/EspressifApp/EsptouchForAndroid/blob/master/src/com/espressif/iot/esptouch/protocol/DataCode.java
            //
            // one data format:(data code should have 2 to 65 data)
            // 
            //              control byte    high 4 bits    low 4 bits 
            // 1st 9bits:       0x0          crc(high)      data(high)
            // 2nd 9bits:       0x1             sequence header
            // 3rd 9bits:       0x0          crc(low)       data(low)
            // 
            // sequence header: 0,1,2,...
            //
            packetLengths.push( extraLength + (           (sum & 0xF0)       | (b >> 4 ) ));
            packetLengths.push( extraLength + ((1 << 8) | (          seq & 0xFF        ) ));
            packetLengths.push( extraLength + (           ((sum & 0xF) << 4) | (b & 0xF) ));
        }

        // console.log( packetLengths.map((x) => x.toString(10).padStart(3, ' ')).join('\n') );
        return packetLengths;
    }

    /////////////////////////////////////////////////////////////////////////
    /// start sending out Smart Config packets
    /// https://github.com/EspressifApp/EsptouchForAndroid/blob/master/src/com/espressif/iot/esptouch/task/EsptouchTaskParameter.java
    /// https://github.com/EspressifApp/EsptouchForAndroid/blob/master/src/com/espressif/iot/esptouch/task/__EsptouchTask.java
    async beginAirKiss_() {
        const packetLengths = this.airKissPacketLengths_();

        // delay for sec seconds
        const delay = async (sec) => {
            await new Promise((resolve) => setTimeout(resolve, sec * 1000));
        };

        // guide codes
        const guideCode = [ 515, 514, 513, 512 ];
        const guideCodeSecs = 2.0;
        const guideCodeInterval = 0.008;
        const payloadInterval = 0.008;

        for (;;) {
            // send out guide codes
            const startGuide = elapsedSeconds();
            while ((elapsedSeconds() - startGuide) < guideCodeSecs) {
                for (const i in guideCode) {
                    this.udpSocket_.send('1'.repeat(guideCode[i]), this.multicastPort_, this.mutlicastAddr_);
                    await delay(guideCodeInterval);
                }
            }

            // followed by our payload repeated a couple of times
            for (let repeat = 0; repeat < 3; ++repeat) {
                for (const i in packetLengths) {
                    const len = packetLengths[i];
                    this.udpSocket_.send('1'.repeat(len), this.multicastPort_, this.mutlicastAddr_);
                    await delay(payloadInterval);
                }
                await delay(0.5);
            }
            await delay(0.5);
        }
    }

    /////////////////////////////////////////////////////////////////////////
    /// connected to specified device
    async connectToDevice_(ipDevice) {
        let client = null;
        try {
            // await new Promise((dataResolve, dataReject) => {
            client = await new Promise((resolve) => {
                const c = net.createConnection(
                    this.deviceConfigPort_, ipDevice,
                    () => resolve(c)
                );
            });
            console.log(`Connected to device ${ipDevice}`);

            let clientResponse = null;
            let clientData = Buffer.alloc(0);
            client.on('data', (data) => {
                clientData = Buffer.concat([clientData, data]);

                let len = 1 + clientData[0];
                if (clientData.length <= len) return; // wait for rest of packet

                let n = clientData.indexOf(0); // check for null terminator
                n = (n >= 0 && n < len) ? n : len;
                const body = clientData.slice(1, n).toString('utf8');

                clientData = Buffer.from(clientData.slice(len));
                if (clientData.length > 0 && 0 == clientData[0]) clientData = clientData.slice(1);

                if (clientResponse) {
                    clientResponse.resolve(JSON.parse(body));
                } else {
                    console.log(JSON.parse(body));
                }
            });
            client.on('end', (e) => {
                if (clientResponse) clientResponse.reject(e);
            });

            const clientSend = (json) => {
                return new Promise((resolve) => {
                    const msg = JSON.stringify(json);
                    // console.log('<=', msg);
                    client.write( Buffer.concat([
                        Buffer.from([ msg.length ]),
                        Buffer.from(msg),
                    ]), resolve);
                });
            };

            // trigger configure
            await clientSend({
                'uri' : '/beginConfigRequest',
                'wifiID' : this.apSsid_,
                'wifiBssid': '',
                'wifiPassword' : this.apPass_,
                'account' : '0',
                'key' : Date.now(),
                'serverIP' : this.ipAddress_,
            });

            // waiting for response
            const response = await new Promise((resolve, reject) => { clientResponse = { resolve, reject }; });
            console.log(response);

        } finally {
            if (client) client.end();
            client = null;
        }
    }

    /////////////////////////////////////////////////////////////////////////
    /// spin up our HTTP server
    /// handles static requests + WebSocket
    beginHttpServer_() {
        // rolling our own HTTP handler
        // was having difficulty with express' handling of vesync requests :(
        const staticPath = path.join(__dirname, 'assets');
        const httpLogger = morgan('combined');
        this.httpServer_ = http.createServer((req, res) => {
            httpLogger(req, res, () => {});

            // determine path, checking that it doesn't escape
            let filePath = path.resolve(path.join(staticPath, req.url));
            if (!filePath.startsWith(staticPath)) {
                res.writeHead(404);
                res.end();
                return;
            }

            // stat requested file
            let stat = null;
            try {
                stat = fs.statSync(filePath);
                if (stat.isDirectory()) {
                    // index fallback
                    filePath = path.join(filePath, 'index.html');
                    stat = fs.statSync(filePath);
                }
            } catch (e) {
                stat = null;
            }

            //
            if (!stat || !stat.isFile()) {
                res.writeHead(404);
                res.end();
                return;
            }

            try {
                if ('GET' === req.method || 'HEAD' === req.method) {
                    res.writeHead(200, {
                        'Content-Length': stat.size
                    });

                    if ('GET' === req.method) {
                        // https://stackoverflow.com/questions/10046039/nodejs-send-file-in-response
                        fs.createReadStream(filePath).pipe(res);
                    } else {
                        res.end();
                    }
                } else {
                    res.writeHead(400);
                    res.end();
                }

            } catch (e) {
                console.error(e);
                res.writeHead(500);
                res.end();
                return;
            }
        });


        // attach WebSocket handler
        this.websocketServer_ = new WebSocket.Server({
            path: '/gnws',
            server: this.httpServer_,
        });
        this.websocketServer_.on('connection', (ws, req) => {
            const remoteAddress = req.connection.remoteAddress;
            console.log(`Accepted WebSocket from ${remoteAddress}`);
            
            const aesKey = 'llwantaeskey1.01';
            const aesIv = 'llwantaesivv1.01';
            let wsEncrypted = false;

            const wsSendMessage = (msg) => {
                let data;
                if (wsEncrypted) {
                    const cipher = crypto.createCipheriv('aes-128-cbc', aesKey, aesIv);
                    cipher.setAutoPadding(false);
                    data = cipher.update(msg + '\0'.repeat(16 - msg.length % 16));
                } else {
                    data = msg;
                }
                ws.send(data);
            };

            ws.on('message', (data) => {
                try {
                    const msg = (() => {
                        if (data instanceof Buffer) {
                            wsEncrypted = true;

                            // decrypt
                            const decipher = crypto.createDecipheriv('aes-128-cbc', aesKey, aesIv);
                            decipher.setAutoPadding(false);
                            const d = decipher.update(data);

                            const idx = d.findIndex((c) => (0 === c));
                            return d.slice(0, (idx > 0) ? idx : undefined);
                        } else {
                            return data;
                        }
                    })();

                    const json = JSON.parse(msg);
                    console.log(json);

                    // ignore uri requests
                    if (null != json.uri) return;

                    // verify some device details before we initiate an upgrade
                    if (   'vesync_wifi_outlet' !== json.deviceName
                        || 'wifi-switch' !== json.type)
                    {
                        throw new Error(`Unexpected device ${json.deviceName} ${json.type}`);
                    }

                    // login success
                    const d = new Date();
                    wsSendMessage(JSON.stringify({
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

                    if (!ws.sentUpgrade_) {
                        console.log('Initiating device upgrade');
                        ws.sentUpgrade_ = true;
                        wsSendMessage(JSON.stringify({
                            uri: '/upgrade',
                            url: `http://${this.ipAddress_}:${this.httpPort_}`,
                            newVersion: '2.00',
                        }));
                    }
                } catch (e) {
                    console.error(e);
                }
            });
        });

        //
        console.log(`Starting web server on TCP port ${this.httpPort_}`);
        this.httpServer_.listen(this.httpPort_);
    }
}

// let's begin
const vesyncHijack = new VesyncHijack();
vesyncHijack.begin().catch(console.error);
