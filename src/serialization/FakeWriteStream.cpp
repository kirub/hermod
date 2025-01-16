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
        , CurrentSizeInBits(0)
        , Error(PROTO_ERROR_NONE)
        , AccumulatedBits(0)
    {
    }

    FakeWriteStream::~FakeWriteStream()
    {
    }

    void FakeWriteStream::Reset()
    {
        CurrentSizeInBits = 0;
        AccumulatedBits = 0;
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
        if (AccumulatedBits >= 32)
        {
            CurrentSizeInBits += 32;
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
        int RemainingBytes = std::min(InBytesCount, (4 - (AccumulatedBits % 32) / 8) % 4);
        for (int idx = 0; idx < RemainingBytes; ++idx)
            SerializeBits(Unused, 8);
        
        if (InBytesCount == RemainingBytes)
            return true;

        assert(AccumulatedBits == 0);

        int MaxWords = (InBytesCount - RemainingBytes) / 4;
        CurrentSizeInBits += MaxWords * 32;

        RemainingBytes = InBytesCount - ((MaxWords * 4) + RemainingBytes);
        assert(RemainingBytes < 4);

        for(int idx = 0; idx < RemainingBytes; ++idx)
        {
            SerializeBits(Unused, 8);
        }
        return true;
    }

    bool FakeWriteStream::SerializeAlign(uint32_t AlignToBits /*= 8*/)
    {
        uint32_t unused = 0;
        if (int AlignedBits = GetAlignBits(AlignToBits))
        {
            SerializeBits(unused, AlignedBits);
        }
        return true;
    }

    int FakeWriteStream::GetAlignBits() const
    {
        return GetAlignBits(8);
    }

    int FakeWriteStream::GetAlignBits(uint32_t AlignToBits) const
    {
        return (AlignToBits - (AccumulatedBits % AlignToBits)) % AlignToBits;
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
        CurrentSizeInBits += AccumulatedBits;
        AccumulatedBits = 0;
    }
    
    void FakeWriteStream::EndWrite()
    {
        Flush();
    }

    bool FakeWriteStream::WouldOverflow(int bytes) const
    {
        return CurrentSizeInBits + AccumulatedBits + bytes * 8 > SizeMax;
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
        return (CurrentSizeInBits + 7) / 8;
    }

    int FakeWriteStream::GetBitsProcessed() const
    {
        return CurrentSizeInBits;
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
        return CurrentSizeInBits + AccumulatedBits;
    }

    int FakeWriteStream::GetTotalBytes() const
    {
        return ( CurrentSizeInBits + AccumulatedBits + 7) / 8;
    }

    int FakeWriteStream::GetError() const
    {
        return Error;
    }
}