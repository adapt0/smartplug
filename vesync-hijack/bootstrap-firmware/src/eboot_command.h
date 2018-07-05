/////////////////////////////////////////////////////////////////////////////
/** @file
eboot command

Based on eboot_command.h which is part of the eboot bootloader.
Copyright (c) 2015 Ivan Grokhotkov. All rights reserved. 

Redistribution and use is permitted according to the conditions of the
3-clause BSD license provided with the eboot bootloader source code        */
/////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED__EBOOT_COMMAND
#define INCLUDED__EBOOT_COMMAND

//- includes
#include <cstdint>

namespace Eboot {

auto* RTC_MEM = ((volatile uint32_t*)0x60001200);

enum class Action : uint32_t {
    COPY_RAW = 0x00000001,
    LOAD_APP = 0xffffffff
};

const uint32_t EBOOT_MAGIC      = 0xeb001000;
const uint32_t EBOOT_MAGIC_MASK = 0xfffff000;

/////////////////////////////////////////////////////////////////////////////
/// eboot command
struct Command {
    /////////////////////////////////////////////////////////////////////////
    /// construct an eboot command
    Command(Action action, std::initializer_list<uint32_t> inArgs)
    : magic_(EBOOT_MAGIC)
    , action_(action)
    {
        std::copy(inArgs.begin(), inArgs.end(), args_);
        std::fill(args_ + inArgs.size(), args_ + sizeof(args_)/sizeof(args_[0]), 0u);
        crc32_ = crc32Calculate(this);
    }

    /////////////////////////////////////////////////////////////////////////
    /// calculate crc32 for Command
    static uint32_t crc32Calculate(const Command* cmd) {
        return crc32Update(0xffffffff, (const uint8_t*)cmd, sizeof(Command) - sizeof(uint32_t));
    }
    /// update CRC32
    static uint32_t crc32Update(uint32_t crc, const uint8_t *data, size_t length) {
        while (length--) {
            const uint8_t c = *data++;
            for (uint32_t i = 0x80; i > 0; i >>= 1) {
                bool bit = crc & 0x80000000;
                if (c & i) {
                    bit = !bit;
                }
                crc <<= 1;
                if (bit) {
                    crc ^= 0x04c11db7;
                }
            }
        }
        return crc;
    }

private:
    const uint32_t  magic_ = EBOOT_MAGIC;
    const Action    action_;
    uint32_t        args_[29];
    uint32_t        crc32_{0};
};

/////////////////////////////////////////////////////////////////////////////
/// write eboot command
static inline void writeCommand(const Command& cmd) {
    const uint32_t dw_count = sizeof(Command) / sizeof(uint32_t);
    const uint32_t* src = (const uint32_t*)&cmd;
    for (uint32_t i = 0; i < dw_count; ++i) RTC_MEM[i] = src[i];
}

} // namespace Eboot

#endif // INCLUDED__EBOOT_COMMAND
