#include <hermod/serialization/BitWriter.h>
#include <utility>

namespace serialization
{
    BitWriter::BitWriter(void* data, int bytes)
        : m_data((uint32_t*)data)
        , m_numWords(bytes / 4)
    {
        assert((bytes % 4) == 0);           // buffer size must be a multiple of four
        Reset();
    }


    void BitWriter::Reset()
    {
        m_numBits = m_numWords * 32;
        m_bitsWritten = 0;
        m_wordIndex = 0;
        m_scratch = 0;
        m_scratchBits = 0;
    }

    void BitWriter::Reset(void* data, int bytes)
    {
        m_data = (uint32_t*)data;
        m_numWords = bytes / 4;
        assert((bytes % 4) == 0);           // buffer size must be a multiple of four
        Reset();
    }

    void BitWriter::WriteBits(uint32_t value, int bits)
    {
        assert(bits > 0);
        assert(bits <= 32);
        assert(m_bitsWritten + bits <= m_numBits);

        value &= (uint64_t(1) << bits) - 1;

        m_scratch |= uint64_t(value) << m_scratchBits;

        m_scratchBits += bits;

        if (m_scratchBits >= 32)
        {
            assert(FlushBits());
        }
    }

    void BitWriter::WriteAlign(int BitsMultiple /*= 8*/)
    {
        const int remainderBits = m_scratchBits % BitsMultiple;
        if (remainderBits != 0)
        {
            uint32_t zero = 0;
            WriteBits(zero, BitsMultiple - remainderBits);
            assert((m_bitsWritten % BitsMultiple) == 0);
        }
    }

    void BitWriter::WriteBytes(const uint8_t* data, int bytes)
    {
        assert(GetAlignBits() == 0);
        assert(m_bitsWritten + bytes * 8 <= m_numBits);
        assert((m_bitsWritten % 32) == 0 || (m_bitsWritten % 32) == 8 || (m_bitsWritten % 32) == 16 || (m_bitsWritten % 32) == 24);

        int headBytes = (4 - (m_scratchBits % 32) / 8) % 4;
        if (headBytes > bytes)
            headBytes = bytes;
        for (int i = 0; i < headBytes; ++i)
            WriteBits(data[i], 8);
        if (headBytes == bytes)
            return;

        assert(GetAlignBits() == 0);

        int numWords = (bytes - headBytes) / 4;
        if (numWords > 0)
        {
            assert((m_bitsWritten % 32) == 0);
            memcpy(&m_data[m_wordIndex], data + headBytes, numWords * 4);
            m_bitsWritten += numWords * 32;
            m_wordIndex += numWords;
            m_scratch = 0;
        }

        assert(GetAlignBits() == 0);

        int tailStart = headBytes + numWords * 4;
        int tailBytes = bytes - tailStart;
        assert(tailBytes >= 0 && tailBytes < 4);
        for (int i = 0; i < tailBytes; ++i)
            WriteBits(data[tailStart + i], 8);

        assert(GetAlignBits() == 0);

        assert(headBytes + numWords * 4 + tailBytes == bytes);
    }

    bool BitWriter::FlushBits()
    {
        if (m_scratchBits != 0)
        {
            assert(m_wordIndex < m_numWords);
            m_data[m_wordIndex] = htonl(uint32_t(m_scratch & 0xFFFFFFFF));
            if ((m_scratch >> m_scratchBits) != 0)
            {
                return false;
            }
            m_scratch >>= 32;
            m_bitsWritten += 32;
            m_scratchBits = std::max(m_scratchBits - 32, 0);
            m_wordIndex++;
        }

        return true;
    }


    bool BitWriter::WouldOverflow(int bits) const
    {
        return m_bitsWritten + bits > m_numBits;
    }

    int BitWriter::GetAlignBits(int BitsMultiple /*= 8*/) const
    {
        return (BitsMultiple - (m_scratchBits % BitsMultiple)) % BitsMultiple;
    }

    int BitWriter::GetBitsWritten() const
    {
        return m_bitsWritten;
    }

    int BitWriter::GetBitsAvailable() const
    {
        return m_numBits - m_bitsWritten;
    }

    const uint8_t* BitWriter::GetData() const
    {
        return (uint8_t*)m_data;
    }

    int BitWriter::GetBytesWritten() const
    {
        return (m_bitsWritten + 7) / 8;
    }

    int BitWriter::GetTotalBytes() const
    {
        return m_numWords * 4;
    }
}