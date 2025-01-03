#include <hermod/serialization/WriteStream.h>
#include <hermod/utilities/Error.h>
#include <hermod/utilities/Utils.h>
#include <hermod/utilities/Hash.h>

namespace serialization
{
    WriteStream::WriteStream(int InSizeInBytes)
        : WriteStream(new unsigned char[InSizeInBytes], InSizeInBytes, [](unsigned char* Ptr) { delete[] Ptr; })
    {
    }
    WriteStream::WriteStream(unsigned char* InBuffer, int InSizeInBytes)
        : WriteStream(InBuffer, InSizeInBytes, nullptr)
    {
    }

    WriteStream::WriteStream(unsigned char* InBuffer, int InSizeInBytes, Deleter InDeleter)
        : IStream(Writing)
        , Data(InBuffer)
        , Error(PROTO_ERROR_NONE)
        , Writer(InBuffer, InSizeInBytes)
        , DeleterFunc(InDeleter)
    {
    }

    WriteStream::~WriteStream()
    {
        if (DeleterFunc)
        {
            delete[] Data;
        }
    }

    void WriteStream::Reset()
    {
        Writer.Reset();
        Error = PROTO_ERROR_NONE;
    }

    bool WriteStream::SerializeInteger(int32_t& InValue, int32_t InMin, int32_t InMax)
    {
        assert(InMin < InMax);
        assert(InValue >= InMin);
        assert(InValue <= InMax);
        const int Bits = utils::bits_required(InMin, InMax);
        uint32_t UnsignedValue = InValue - InMin;
        Writer.WriteBits(UnsignedValue, Bits);
        return true;
    }

    bool WriteStream::SerializeBits(uint32_t& InValue, int InBitsCount)
    {
        assert(InBitsCount > 0);
        assert(InBitsCount <= 32);
        Writer.WriteBits(InValue, InBitsCount);
        return true;
    }

    bool WriteStream::SerializeBytes(const uint8_t* InData, int InBytesCount)
    {
        assert(InData);
        assert(InBytesCount >= 0);
        Writer.WriteAlign(8);
        Writer.WriteBytes(InData, InBytesCount);
        return true;
    }

    bool WriteStream::SerializeAlign()
    {
        Writer.WriteAlign();
        return true;
    }

    int WriteStream::GetAlignBits() const
    {
        return Writer.GetAlignBits();
    }

    bool WriteStream::SerializeCheck(const char* string)
    {
#if PROTO_SERIALIZE_CHECKS
        SerializeAlign();
        uint32_t Magic = hash::string(string, 0);
        SerializeBits(Magic, 32);
#endif // #if PROTO_SERIALIZE_CHECKS
        return true;
    }

    void WriteStream::Flush()
    {
        Writer.FlushBits();
    }

    bool WriteStream::WouldOverflow(int bytes) const
    {
        return Writer.WouldOverflow(bytes * 8);
    }

    void WriteStream::EndWrite()
    {
        Flush();
    }

    const uint8_t* WriteStream::GetData()
    {
        Flush();
        return Writer.GetData();
    }

    int WriteStream::GetDataSize() const
    {
        return GetBytesProcessed();
    }

    int WriteStream::GetBytesProcessed() const
    {
        return Writer.GetBytesWritten();
    }

    int WriteStream::GetBitsProcessed() const
    {
        return Writer.GetBitsWritten();
    }

    int WriteStream::GetBitsRemaining() const
    {
        return GetTotalBits() - GetBitsProcessed();
    }
    int WriteStream::GetBytesRemaining() const
    {
        return Writer.GetBitsAvailable() / 8;
    }

    int WriteStream::GetTotalBits() const
    {
        return Writer.GetTotalBytes() * 8;
    }

    int WriteStream::GetTotalBytes() const
    {
        return Writer.GetTotalBytes();
    }

    int WriteStream::GetError() const
    {
        return Error;
    }
}