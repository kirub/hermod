#include <hermod/serialization/ReadStream.h>
#include <hermod/utilities/Utils.h>
#include <hermod/utilities/Error.h>
#include <hermod/utilities/Hash.h>

namespace serialization
{
    ReadStream::ReadStream(int InSizeInBytes)
        : ReadStream(new unsigned char[InSizeInBytes], InSizeInBytes, [](unsigned char* Ptr) { delete[] Ptr; })
    {
    }
    
    ReadStream::ReadStream(unsigned char* InBuffer, int InSizeInBytes, Deleter InDeleter)
        : IStream(Reading)
        , Reader(InBuffer, InSizeInBytes)
        , Error(PROTO_ERROR_NONE)
        , BitsRead(0)
        , DeleterFunc(InDeleter)
    {


    }

    ReadStream::ReadStream(unsigned char* InBuffer, int InSizeInBytes)
        : ReadStream(InBuffer, InSizeInBytes, nullptr)
    {
    }

    ReadStream::~ReadStream()
    {
        if (DeleterFunc)
        {
            DeleterFunc((unsigned char*)Reader.GetData());
        }
    }

    void ReadStream::Reset()
    {
        Reader.Reset();
        Error = PROTO_ERROR_NONE;
        BitsRead = 0;
    }

    bool ReadStream::SerializeInteger(int32_t& OutValue, int32_t InMin, int32_t InMax)
    {
        assert(InMin < InMax);
        const int bits = utils::bits_required(InMin, InMax);
        if (Reader.WouldOverflow(bits))
        {
            Error = PROTO_ERROR_STREAM_OVERFLOW;
            return false;
        }
        uint32_t unsigned_value = Reader.ReadBits(bits);
        OutValue = (int32_t)unsigned_value + InMin;
        BitsRead += bits;
        return true;
    }

    bool ReadStream::SerializeBits(uint32_t& OutVvalue, int InBitsCount)
    {
        assert(InBitsCount > 0);
        assert(InBitsCount <= 32);
        if (Reader.WouldOverflow(InBitsCount))
        {
            Error = PROTO_ERROR_STREAM_OVERFLOW;
            return false;
        }
        uint32_t ReadValue = Reader.ReadBits(InBitsCount);
        OutVvalue = ReadValue;
        BitsRead += InBitsCount;
        return true;
    }

    bool ReadStream::SerializeBytes(const uint8_t* OutData, int InBytesCount)
    {
        if (!SerializeAlign())
            return false;
        if (Reader.WouldOverflow(InBytesCount * 8))
        {
            Error = PROTO_ERROR_STREAM_OVERFLOW;
            return false;
        }
        Reader.ReadBytes((uint8_t*)OutData, InBytesCount);
        BitsRead += InBytesCount * 8;
        return true;
    }

    bool ReadStream::SerializeAlign()
    {
        const int AlignBits = Reader.GetAlignBits();
        if (Reader.WouldOverflow(AlignBits))
        {
            Error = PROTO_ERROR_STREAM_OVERFLOW;
            return false;
        }
        if (!Reader.ReadAlign())
            return false;
        BitsRead += AlignBits;
        return true;
    }

    int ReadStream::GetAlignBits() const
    {
        return Reader.GetAlignBits();
    }

    bool ReadStream::SerializeCheck(const char* InString)
    {
#if PROTO_SERIALIZE_CHECKS            
        SerializeAlign();
        uint32_t Value = 0;
        SerializeAlign();
        SerializeBits(Value, 32);
        const uint32_t Magic = hash::string(InString, 0);
        if (Magic != Value)
        {
            printf("serialize check failed: '%s'. expected %x, got %x\n", InString, Magic, Value);
        }
        return Value == Magic;
#else // #if PROTO_SERIALIZE_CHECKS
        return true;
#endif // #if PROTO_SERIALIZE_CHECKS
    }

    const uint8_t* ReadStream::GetData()
    {
        return Reader.GetData();
    }

    int ReadStream::GetDataSize() const
    {
        return GetBytesProcessed();
    }

    int ReadStream::GetBitsProcessed() const
    {
        return BitsRead;
    }

    int ReadStream::GetBitsRemaining() const
    {
        return Reader.GetBitsRemaining();
    }
    int ReadStream::GetBytesRemaining() const
    {
        return Reader.GetBitsRemaining() / 8;
    }

    int ReadStream::GetBytesProcessed() const
    {
        return (BitsRead + 7) / 8;
    }

    int ReadStream::GetError() const
    {
        return Error;
    }

    int ReadStream::GetBytesRead() const
    {
        return Reader.GetBytesRead();
    }
}