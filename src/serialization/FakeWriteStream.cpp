#include <hermod/serialization/FakeWriteStream.h>
#include <hermod/utilities/Error.h>
#include <hermod/utilities/Utils.h>
#include <hermod/utilities/Hash.h>

namespace serialization
{
    bool FakeWriteStream::SimulateBitpacker = true;

    FakeWriteStream::FakeWriteStream(int InSizeInBytes)
        : IStream(Writing)
        , SizeMax(InSizeInBytes)
        , CurrentSize(0)
        , Error(PROTO_ERROR_NONE)
    {
    }

    FakeWriteStream::~FakeWriteStream()
    {
    }

    void FakeWriteStream::Reset()
    {
        CurrentSize = 0;
        Error = PROTO_ERROR_NONE;
    }

    bool FakeWriteStream::SerializeInteger(int32_t& InValue, int32_t InMin, int32_t InMax)
    {
        assert(InMin < InMax);
        assert(InValue >= InMin);
        assert(InValue <= InMax);
        const int Bits = utils::bits_required(InMin, InMax);
        uint32_t Unused = 0;
        return SerializeBits(Unused, Bits);
    }

    bool FakeWriteStream::SerializeBits(uint32_t& InValue, int InBitsCount)
    {
        assert(InBitsCount > 0);
        assert(InBitsCount <= 32);
        AccumulatedBits += InBitsCount;
        if (AccumulatedBits > 32)
        {
            CurrentSize += 32;
            AccumulatedBits = AccumulatedBits % 32;
        }
        return true;
    }

    bool FakeWriteStream::SerializeBytes(const uint8_t* InData, int InBytesCount)
    {
        assert(InData);
        assert(InBytesCount >= 0);
        if (!SerializeAlign())
            return false;


        assert((AccumulatedBits % 8) == 0);

        uint32_t Unused = 0;
        int RemainingBytes = std::min(InBytesCount, (4 - (CurrentSize % 32)) % 4);
        for (int idx = 0; idx < RemainingBytes; ++idx)
            SerializeBits(Unused, 8);
        
        if (InBytesCount == RemainingBytes)
            return true;

        assert(AccumulatedBits == 0);

        int MaxWords = (InBytesCount - RemainingBytes) / 4;
        CurrentSize += MaxWords * 4;

        RemainingBytes = InBytesCount - (MaxWords * 4);
        assert(RemainingBytes < 4);

        for(int idx = 0; idx < RemainingBytes; ++idx)
        {
            SerializeBits(Unused, 8);
        }
        return true;
    }

    bool FakeWriteStream::SerializeAlign()
    {
        uint32_t unused = 0;
        SerializeBits(unused, GetAlignBits());
        return true;
    }

    int FakeWriteStream::GetAlignBits() const
    {
        return (8 - (CurrentSize % 8)) % 8;
    }

    bool FakeWriteStream::SerializeCheck(const char* string)
    {
#if PROTO_SERIALIZE_CHECKS
        SerializeAlign();
        uint32_t Magic = hash::string(string, 0);
        SerializeBits(Magic, 32);
#endif // #if PROTO_SERIALIZE_CHECKS
        return true;
    }

    void FakeWriteStream::Flush()
    {
        CurrentSize += AccumulatedBits;
        AccumulatedBits = 0;
    }
    
    void FakeWriteStream::EndWrite()
    {
        Flush();
    }

    bool FakeWriteStream::WouldOverflow(int bytes) const
    {
        return CurrentSize + AccumulatedBits + bytes * 8 > SizeMax;
    }

    const uint8_t* FakeWriteStream::GetData()
    {
        return nullptr;
    }

    int FakeWriteStream::GetDataSize() const
    {
        return GetBytesProcessed();
    }

    int FakeWriteStream::GetBytesProcessed() const
    {
        return (CurrentSize + 7) / 8;
    }

    int FakeWriteStream::GetBitsProcessed() const
    {
        return CurrentSize;
    }

    int FakeWriteStream::GetBitsRemaining() const
    {
        return GetTotalBits() - GetBitsProcessed();
    }
    int FakeWriteStream::GetBytesRemaining() const
    {
        return (SizeMax - GetBitsProcessed()) / 8;
    }

    int FakeWriteStream::GetTotalBits() const
    {
        return CurrentSize + AccumulatedBits;
    }

    int FakeWriteStream::GetTotalBytes() const
    {
        return ( CurrentSize + AccumulatedBits + 7) / 8;
    }

    int FakeWriteStream::GetError() const
    {
        return Error;
    }
}