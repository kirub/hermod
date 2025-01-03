#pragma once

#include <hermod/platform/Platform.h>

#include <cstdint>
#include <cassert>

namespace serialization
{
    class HERMOD_API BitReader
    {
    public:

        BitReader(const void* data, int bytes);
        void Reset();

        bool WouldOverflow(int bits) const;

        void SetSize(int InNumBytes);
        uint32_t ReadBits(int bits);
        bool ReadAlign(int BitsMultiple = 8);
        void ReadBytes(uint8_t* data, int bytes);


        const uint8_t* GetData() const;
        int GetAlignBits(int BitsMultiple = 8) const;
        int GetBitsRead() const;
        int GetBytesRead() const;
        int GetBitsRemaining() const;
        int GetBytesRemaining() const;
        int GetTotalBits() const;
        int GetTotalBytes() const;

    private:

        const uint32_t* m_data;
        uint64_t m_scratch;
        int m_capacityBytes;
        int m_numBits;
        int m_numBytes;
    #ifdef DEBUG
        int m_numWords;
    #endif // #ifdef DEBUG
        int m_bitsRead;
        int m_scratchBits;
        int m_wordIndex;
};
}