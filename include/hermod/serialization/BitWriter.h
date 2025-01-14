#pragma once

#include <hermod/platform/Platform.h>

#include <cstdint>
#include <cassert>
#include <winsock2.h>

namespace serialization
{
    class HERMOD_API BitWriter
    {
    public:
        BitWriter(void* data, int bytes);

        void WriteBits(uint32_t value, int bits);
        void WriteAlign(int BitsMultiple = 8);
        void WriteBytes(const uint8_t* data, int bytes);

        void Reset();
        void FlushBits();

        bool WouldOverflow(int bits) const;

        int GetAlignBits(int BitsMultiple = 8) const;
        int GetBitsWritten() const;
        int GetBitsAvailable() const;
        const uint8_t* GetData() const;
        int GetBytesWritten() const;
        int GetTotalBytes() const;

    private:

        uint32_t* m_data;
        uint64_t m_scratch;
        int m_numBits;
        int m_numWords;
        int m_bitsWritten;
        int m_wordIndex;
        int m_scratchBits;
    };
}