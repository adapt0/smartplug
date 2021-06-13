#!/usr/bin/env python
# Copyright (c) 2018 Chris Byrne. All rights reserved.
# Licensed under the MIT License. Refer to LICENSE file in the project root.

import base64
import binascii
import os
import re
import struct
import subprocess
import sys

Import("env")

# Dump build environment (for debug)
# print env.Dump()

# print "Current build targets", map(str, BUILD_TARGETS)


# convert custom options to dictionary
# https://community.platformio.org/t/custom-option-for-extra-script-encryption/909/9
args = dict()
customOption = ARGUMENTS.get("CUSTOM_OPTION")
if customOption:
    customOption = base64.b64decode(ARGUMENTS.get("CUSTOM_OPTION"))
    for m in re.finditer(R"""([^\s=]+)=\s?([^\s,]+)""", customOption):
        args[m.group(1)] = m.group(2)

# print args


def elf_to_user_bin(source, target, env):
    """Extracts & partitions sections from an ELF file, to create an ESP8266
    SDK OTA friendly binary
    """

    # add path to readelf tool if it's missing from our environment
    if 'READELF' not in env:
        env.AppendUnique(READELF=env['RANLIB'].replace('ranlib', 'readelf'))

    # print(env.Dump())

    #  SPI FLASH PARAMS
    # -------------------
    # flash_mode=
    #     0: QIO
    #     1: QOUT
    #     2: DIO
    #     3: DOUT
    flash_mode = {
        "qio": 0,
        "qout": 1,
        "dio": 2,
        "dout": 3,
    }[env['BOARD_FLASH_MODE'].lower()]

    # flash_clk_div=
    #     0 :  80m / 2
    #     1 :  80m / 3
    #     2 :  80m / 4
    #    0xf:  80m / 1
    # flash_size_map=
    #     0 : 512 KB (256 KB + 256 KB)
    #     1 : 256 KB
    #     2 : 1024 KB (512 KB + 512 KB)
    #     3 : 2048 KB (512 KB + 512 KB)
    #     4 : 4096 KB (512 KB + 512 KB)
    #     5 : 2048 KB (1024 KB + 1024 KB)
    #     6 : 4096 KB (1024 KB + 1024 KB)

    # byte3=(((int(flash_size_map)<<4)| int(flash_clk_div))&0xff)

# 'BOARD_F_CPU': u'80000000L',
# 'BOARD_F_FLASH': u'40000000L',
    flash_freq = 64

    user_mode = int(env['USERMODE'])

    path_elf = str(target[0])
    path_bin = os.path.splitext(path_elf)[0] + '.bin'

    # retrieve section headers
    headers = subprocess.check_output([
        env['READELF'],
        '--headers',
        path_elf,
    ], env=env['ENV'], universal_newlines=True)

    # print(headers)

    # Entry point address:               0x40100004
    entry_point = int(re.search(r"""Entry point address:\s+0x([0-9a-f]+)""", headers).group(1), 16)

    # sections
    sections = dict()
    section_re = re.compile(
        # [Nr] Name              Type            Addr     Off    Size   ES Flg Lk Inf Al
        # [ 4] .irom0.text       PROGBITS        40281010 006930 0302a8 00  AX  0   0 16
        r"""\[ ?\d+\] (\.\S+)\s+\S+\s+([0-9a-f]+) ([0-9a-f]+) ([0-9a-f]+)""",
        re.MULTILINE
    )
    for sm in re.finditer(section_re, headers):
        sections[sm.group(1)] = {
            'address': int(sm.group(2), 16),
            'offset': int(sm.group(3), 16),
            'size': int(sm.group(4), 16),
        }

    f_bin = None
    f_elf = None
    try:
        f_elf = open(path_elf, 'rb')
        f_bin = open(path_bin, 'w+b')

        rom_section = sections['.irom0.text']
        rom_length = rom_section['size']
        section_length = (rom_length + 15) & ~0xF

        # sanity check that we're using the right linker scripts
        if 1 == int(env['USERMODE']):
            assert rom_section['address'] == 0x40201010, "irom0 at unexpected offset 0x{:x} (expected 0x{:x}) - wrong linker script?)".format(rom_section['address'], 0x40201010)
        else:
            assert rom_section['address'] == 0x40281010, "irom0 at unexpected offset 0x{:x} (expected 0x{:x}) - wrong linker script?)".format(rom_section['address'], 0x40281010)

        # irom header
        f_bin.write(struct.pack(
            '<BBBBLLL',
            0xea,           # uint8 magic1
            0x04,           # uint8 magic2
            0x00,           # uint8 unknown
            user_mode,      # uint8 user mode 1/2
            entry_point,    # uint32 entry point
            0,              # uint32 unused?
            section_length  # uint32 section length
        ))

        f_elf.seek(rom_section['offset'])
        f_bin.write(f_elf.read(rom_length))
        if section_length > rom_length:
            f_bin.write(bytearray([0] * (section_length - rom_length)))  # padding

        # Partition map
        partitions = ['.text', '.data', '.rodata']
        f_bin.write(struct.pack(
            '<BBBBL',
            0xe9,               # uint8 Magic
            len(partitions),    # uint8 Segments
            flash_mode,         # uint8 Flash mode
            flash_freq,         # uint8 Flash size frequency
            entry_point,        # uint32 Entry point address
        ))

        checksum = 0xEF
        for name in partitions:
            rom_section = sections[name]
            rom_length = rom_section['size']

            f_bin.write(struct.pack(
                '<LL',
                rom_section['address'],  # Destination address
                rom_length,              # Segment length
            ))

            # print(name, romSection['address'])
            f_elf.seek(rom_section['offset'])
            rom_data = f_elf.read(rom_length)
            f_bin.write(rom_data)

            if sys.version_info < (3,):
                for r in rom_data:
                    checksum ^= ord(r)
            else:
                for r in rom_data:
                    checksum ^= r

        pad = 16 - (f_bin.tell() % 16) - 1
        f_bin.write(bytearray([0] * pad))

        f_bin.write(struct.pack('B', checksum & 0xFF))

        # calc "CRC32" (per Espressif's SDK)
        f_bin.seek(0)
        block_size = 1024 * 64
        crc = 0
        while True:
            d = f_bin.read(block_size)
            if 0 == len(d):
                break
            crc = binascii.crc32(d, crc)
        if crc < 0:
            crc = abs(crc) - 1
        else:
            crc = abs(crc) + 1

        f_bin.write(struct.pack('<L', crc))

    except Exception as e:
        print("Unexpected error:", e, sys.exc_info()[0])
        raise

    finally:
        if f_bin:
            f_bin.close()
        if f_elf:
            f_elf.close()


def add_user_target(elf_target, user_mode):
    """add a user partition binary target based on an existing compilation target
    i.e., my_app.elf -> user1.elf + user1.bin

    :param elf_target: (File) SCons compilation elf target
    :param user_mode: (int) 1 for user1, 2 for user2. Dictates the applicable linker script
    """

    linker_script = '../ld/eagle.app.v6.new.1024.app{}.ld'.format(user_mode)

    e = elf_target.get_build_env().Clone(
        LDSCRIPT_PATH=linker_script,
        USERMODE=str(user_mode)
    )

    # HACK replace linker script with desired partitioned one
    # HACK this seems hacky, not sure the correct way to be doing this?
    link_flags = e.get('LINKFLAGS')
    idx = link_flags.index('-T')
    link_flags[idx + 1] = linker_script

    program = e.Program(
        os.path.join(os.path.dirname(elf_target.get_path()), "user{}.elf".format(user_mode)),
        elf_target.sources
    )
    Depends(program, elf_target)

    BUILD_TARGETS.append(program[0])
    env.AddPostAction(program[0], elf_to_user_bin)
    env.AlwaysBuild(env.Alias("user{}".format(user_mode), None, program))


# add user1 + user2 outputs dependent on the main application target
build_target = env.File("$BUILD_DIR/${PROGNAME}.elf")
add_user_target(build_target, 1)  # user1
add_user_target(build_target, 2)  # user2
