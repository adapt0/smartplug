/////////////////////////////////////////////////////////////////////////////
/** @file
esp8266 binary image explorer

https://www.pushrate.com/blog/articles/esp8266_boot
https://github.com/pfalcon/esp8266-re-wiki-mirror/blob/master/SPI_Flash_Format.mw

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////

const commander = require('commander');
const fs = require('fs');

/////////////////////////////////////////////////////////////////////////////
/// calculate "CRC32" the same way as Espressif's SDK
/// where gen_appbin has some signed numbered games after the crc32 calculation
const espCrc32 = (() => {
    /// CRC32 calculations
    /// https://stackoverflow.com/questions/18638900/javascript-crc32
    const crc32 = (() => {
        const table = new Array(256);
        for (let n = 0; n < 256; ++n) {
            let c = n;
            for (let k = 0; k < 8; ++k) {
                c = ((c & 1) ? (0xEDB88320 ^ (c >>> 1)) : (c >>> 1));
            }
            table[n] = c;
        }

        function calc(bytes) {
            // special case for string
            if ('string' === typeof(bytes)) return calc(Buffer.from(bytes));

            let crc = 0 ^ (-1);
            for (var i = 0; i < bytes.length; i++ ) {
                crc = (crc >>> 8) ^ table[(crc ^ bytes[i]) & 0xFF];
            }
            return (crc ^ (-1)) >>> 0;
        }
        return calc;
    })();

    /// Espressif CRC32 calculation
    return function(bytes) {
        const c = crc32(bytes);
        return (c < 0x80000000) ? c + 1 : 0xFFFFFFFF - c;
    };
})();

/////////////////////////////////////////////////////////////////////////////
// parse command line
commander
    .version(require('./package.json').version)
    .arguments('<bin...>')
    .option('--extract', 'Extract partitions into separate files')
    .parse(process.argv)
;
if (0 == commander.args.length) commander.help();


for (let filename of commander.args) {
    console.log(`=== ${filename} ===`);

    try {
        const buf = fs.readFileSync(filename);
        console.log(`Size: 0x${buf.length.toString(16)}`);
        for (let ofs = 0; ofs < buf.length; ) {
            console.log(`[0x${buf[ofs].toString(16)}] @ 0x${ofs.toString(16)}`);

            if (0xe9 === buf[ofs]) {
                // old style
                // Partition map
                //  uint8  Magic    0xE9
                //  uint8  Segments
                //  uint8  Flash mode
                //  uint8  Flash size frequency
                //  uint32 Entry point address
                const segments = buf[ofs + 1];
                const flashMode = buf[ofs + 2];
                const flashFreq = buf[ofs + 3];
                const entry = buf[ofs + 4] | buf[ofs + 5] << 8 | buf[ofs + 6] << 16 | buf[ofs + 7] << 24;

                console.log(` Segments: ${segments} Flash mode: ${flashMode} Flash freq: ${flashFreq}`);
                console.log(` Entry: 0x${entry.toString(16)}`);

                let csumCalculated = 0xEF;
                ofs += 8;
                for (let segment = 0; segment < segments; ++segment) {
                    // uint32 Destination address
                    // uint32 Segment length
                    const dest = buf[ofs + 0] | buf[ofs + 1] << 8 | buf[ofs + 2] << 16 | buf[ofs + 3] << 24;
                    const slen = buf[ofs + 4] | buf[ofs + 5] << 8 | buf[ofs + 6] << 16 | buf[ofs + 7] << 24;
                    ofs += 8;

                    console.log(` Segment ${segment} 0x${ofs.toString(16)} Load 0x${dest.toString(16)} ${slen} bytes`);

                    const segmentData = buf.slice(ofs, ofs + slen);
                    if (commander.extract) fs.writeFileSync(`${filename}-${segment}`, segmentData);

                    csumCalculated = segmentData.reduce((x, out) => out ^ x, csumCalculated);

                    ofs += slen;
                }

                // pad out
                const pad = 16 - (ofs % 16);
                ofs += pad;

                // verify checksum
                const csumExpected = buf[ofs - 1];
                if (csumExpected === csumCalculated) {
                    console.log(` csum OK (0x${csumExpected.toString(16)})`);
                } else {
                    console.log(` !!!! csum mismatch! (0x${csumCalculated.toString(16)} != 0x${csumExpected.toString(16)})`);
                }

                // 1k alignment (limit to binary size - crc32)
                const ofsEnd = buf.length - 4;
                ofs = Math.min(ofsEnd, ofs + 0x1000 - (ofs % 0x1000));
                // console.log(` ofs: 0x${ofs.toString(16)}`, buf.slice(ofs));

            } else if (0xea === buf[ofs]) {
                // new style
                // uint8 magic1;    // 0xea
                // uint8 magic2;    // 0x04
                // uint8 config[2];
                // uint32 entry;
                // uint8 unused[4];
                // uint32 length;
                if (0xea !== buf[ofs + 0]) throw new Error('Bad magic1');
                if (0x04 !== buf[ofs + 1]) throw new Error('Bad magic2');

                const config = buf[ofs +  2] | buf[ofs +  3] << 8;
                const entry  = buf[ofs +  4] | buf[ofs +  5] << 8 | buf[ofs +  6] << 16 | buf[ofs +  7] << 24;
                const unused = buf[ofs +  8] | buf[ofs +  9] << 8 | buf[ofs + 10] << 16 | buf[ofs + 11] << 24;
                const length = buf[ofs + 12] | buf[ofs + 13] << 8 | buf[ofs + 14] << 16 | buf[ofs + 15] << 24;

                console.log(` entry: 0x${entry.toString(16)} length: 0x${length.toString(16)} config: 0x${config.toString(16)} unused: 0x${unused.toString(16)}`);

                ofs += 16;
                if (commander.extract) fs.writeFileSync(`${filename}-irom`, buf.slice(ofs, ofs + length));

                ofs += length;

                if (ofs & 0xF) {
                    console.log(' !!!! irom section is not 16 byte aligned');
                }

            } else if (0xff === buf[ofs]) {
                // assume end
                break;

            } else {
                console.log(`Z1 ofs: 0x${ofs.toString(16)}`, buf.slice(ofs));
                throw new Error(`Bad magic ${buf[ofs]}`);
            }

            // trailing CRC?
            if (ofs === (buf.length - 4)) {
                const crcExpected = buf[ofs + 0] | buf[ofs + 1] << 8 | buf[ofs + 2] << 16 | buf[ofs + 3] << 24;
                const crcCalculated = espCrc32(buf.slice(0, ofs));
                if (crcExpected === crcCalculated) {
                    console.log(` crc OK (0x${crcExpected.toString(16)})`);
                } else {
                    console.log(` !!!! crc mismatch! (0x${crcCalculated.toString(16)} != 0x${crcExpected.toString(16)})`);
                }

                ofs += 4;
            }
        }
        console.log('');
    } catch (e) {
        console.error(e.message);
    }
}
