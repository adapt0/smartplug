/////////////////////////////////////////////////////////////////////////////
/** @file
SPI flash writer wrapper

Handles limitations of the SDK's SPI writing
* buffer in RAM must be 4-byte aligned
* size must be 4-byte aligned
* address in flash must be 4-byte aligned

\copyright Copyright (c) 2018 Chris Byrne. All rights reserved.
Licensed under the MIT License. Refer to LICENSE file in the project root. */
/////////////////////////////////////////////////////////////////////////////
#ifndef INCLUDED__FLASH_WRITER
#define INCLUDED__FLASH_WRITER

//- includes
#include <algorithm>
#include <esp_common.h>

/////////////////////////////////////////////////////////////////////////////
/// SPI flash writer wrapper
class FlashWriter {
public:
    /////////////////////////////////////////////////////////////////////////
    /// constructor
    /// @param writeOfs Byte offset in flash to begin writing to
    explicit FlashWriter(size_t writeOfs)
    : writeOfs_(writeOfs)
    {
        buffer_ = new uint32_t[bufferSize_ / sizeof(uint32_t)];
    }
    /// destructor
    ~FlashWriter() {
        delete [] buffer_;
    }

    /////////////////////////////////////////////////////////////////////////
    /// Write a block of data to the flash
    ///
    /// Due to alignment requirements, writes are buffered, with data written
    /// to flash when SPI_FLASH_SEC_SIZE is available.
    /// @sa spiWriteFinal() to pad & finish writing provided blocks to flash
    /// @param data Block of data to write
    /// @param dataLen Size of data to write
    /// @returns SPI_FLASH_RESULT_OK on success
    SpiFlashOpResult spiWrite(const void* data, size_t dataLen) {
        for (size_t ofs = 0; ofs < dataLen; ) {
            const size_t len = std::min(dataLen - ofs, bufferSize_ - bufferOfs_);

            memcpy((uint8_t*)buffer_ + bufferOfs_, (const uint8_t*)data + ofs, len);
            bufferOfs_ += len;
            ofs += len;

            if (bufferOfs_ >= bufferSize_) {
                const auto res = writeBuffer_();
                if (SPI_FLASH_RESULT_OK != res) return res;
            }
        }
        return SPI_FLASH_RESULT_OK;
    }
    /// Complete writing of data to flash
    /// @returns SPI_FLASH_RESULT_OK on success
    SpiFlashOpResult spiWriteFinal() {
        if (0 == bufferOfs_) return SPI_FLASH_RESULT_OK;

        // pad out remaining bytes
        memset((uint8_t*)buffer_ + bufferOfs_, 0xFF, bufferSize_ - bufferOfs_);
        bufferOfs_ = bufferSize_;

        return writeBuffer_();
    }

private:
    /////////////////////////////////////////////////////////////////////////
    /// perform write
    SpiFlashOpResult writeBuffer_() {
        printf("Write 0x%x (%d bytes)\r\n", writeOfs_, bufferOfs_);
        const auto res = spi_flash_write(writeOfs_, buffer_, bufferOfs_);
        if (SPI_FLASH_RESULT_OK != res) return res;

        writeOfs_ += bufferOfs_;
        bufferOfs_ = 0;
        return SPI_FLASH_RESULT_OK;
    }

    uint32_t*       buffer_ = nullptr;                  ///< temporary buffer (32 bit aligned)
    const size_t    bufferSize_ = SPI_FLASH_SEC_SIZE;   ///< size (in bytes) of our buffer)
    size_t          bufferOfs_ = 0;                     ///< byte offset into our buffer
    size_t          writeOfs_ = 0;                      ///< offset we are writing to
};

#endif // INCLUDED__FLASH_WRITER
