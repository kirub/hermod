#include <hermod/serialization/BitReader.h>
#include <hermod/platform/Platform.h>

namespace serialization
{
    BitReader::BitReader(const void* data, int bytes)
        : m_data((const uint32_t*)data)
        , m_capacityBytes(bytes)
        , m_numBytes(bytes)
#ifdef DEBUG
        , m_numWords((bytes + 3) / 4)
#endif // #ifdef DEBUG
    {
        // IMPORTANT: Although we support non-multiples of four bytes passed in, the actual buffer
        // underneath the bit reader must round up to at least 4 bytes because we read a dword at a time.
        assert(data);
        Reset();
    }

    void BitReader::Reset()
    {
        m_numBytes = m_capacityBytes;
        m_numBits = m_numBytes * 8;
        m_bitsRead = 0;
        m_scratch = 0;
        m_scratchBits = 0;
        m_wordIndex = 0;
    }

    void BitReader::SetSize(int InNumBytes)
    {
        m_numBytes = InNumBytes;
        m_numBits = m_numBytes * 8;
    }

    bool BitReader::WouldOverflow(int bits) const
    {
        return m_bitsRead + bits > m_numBits;
    }

    uint32_t BitReader::ReadBits(int bits)
    {
        assert(bits > 0);
        assert(bits <= 32);
        assert(m_bitsRead + bits <= m_numBits);

        m_bitsRead += bits;

        assert(m_scratchBits >= 0 && m_scratchBits <= 64);

        if (m_scratchBits < bits)
        {
#ifdef DEBUG
            assert(m_wordIndex < m_numWords);
#endif
            m_scratch |= uint64_t(ntohl(m_data[m_wordIndex])) << m_scratchBits;
            m_scratchBits += 32;
            m_wordIndex++;
        }

        assert(m_scratchBits >= bits);

        const uint32_t output = m_scratch & ((uint64_t(1) << bits) - 1);

        m_scratch >>= bits;
        m_scratchBits -= bits;

        return output;
    }

    bool BitReader::ReadAlign(int BitsMultiple /*= 8*/)
    {
        const int remainderBits = m_bitsRead % BitsMultiple;
        if (remainderBits != 0)
        {
            uint32_t value = ReadBits(BitsMultiple - remainderBits);
            assert(m_bitsRead % BitsMultiple == 0);
            if (value != 0)
                return false;
        }
        return true;
    }

    void BitReader::ReadBytes(uint8_t* data, int bytes)
    {
        assert(GetAlignBits() == 0);
        assert(m_bitsRead + bytes * 8 <= m_numBits);
        assert((m_bitsRead % 32) == 0 || (m_bitsRead % 32) == 8 || (m_bitsRead % 32) == 16 || (m_bitsRead % 32) == 24);

        int headBytes = (4 - (m_bitsRead % 32) / 8) % 4;
        if (headBytes > bytes)
            headBytes = bytes;
        for (int i = 0; i < headBytes; ++i)
            data[i] = (uint8_t)ReadBits(8);
        if (headBytes == bytes)
            return;

        assert(GetAlignBits() == 0);

        int numWords = (bytes - headBytes) / 4;
        if (numWords > 0)
        {
            assert((m_bitsRead % 32) == 0);
            memcpy(data + headBytes, &m_data[m_wordIndex], numWords * 4);
            m_bitsRead += numWords * 32;
            m_wordIndex += numWords;
            m_scratchBits = 0;
        }

        assert(GetAlignBits() == 0);

        int tailStart = headBytes + numWords * 4;
        int tailBytes = bytes - tailStart;
        assert(tailBytes >= 0 && tailBytes < 4);
        for (int i = 0; i < tailBytes; ++i)
            data[tailStart + i] = (uint8_t)ReadBits(8);

        assert(GetAlignBits() == 0);

        assert(headBytes + numWords * 4 + tailBytes == bytes);
    }

    const uint8_t* BitReader::GetData() const
    {
        return (uint8_t*)m_data;
    }

    int BitReader::GetAlignBits(int BitsMultiple /*= 8*/) const
    {
        return (BitsMultiple - m_bitsRead % BitsMultiple) % BitsMultiple;
    }

    int BitReader::GetBitsRead() const
    {
        return m_bitsRead;
    }

    int BitReader::GetBytesRead() const
    {
        return m_wordIndex * 4;
    }

    int BitReader::GetBitsRemaining() const
    {
        return m_numBits - m_bitsRead;
    }

    int BitReader::GetBytesRemaining() const
    {
        return GetBitsRemaining() / 8;
    }

    int BitReader::GetTotalBits() const
    {
        return m_numBits;
    }

    int BitReader::GetTotalBytes() const
    {
        return m_numBits / 8;
    }
}