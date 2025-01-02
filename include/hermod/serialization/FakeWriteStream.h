#pragma once
#include "Stream.h"
#include "BitWriter.h"

namespace serialization
{
    class HERMOD_API FakeWriteStream :
        public IStream
    {
    public:

        FakeWriteStream(int InSizeInBytes);

        virtual ~FakeWriteStream();
        virtual void Reset();
        virtual const uint8_t* GetData();
        virtual int GetDataSize() const;

        virtual bool WouldOverflow(int bytes) const override;

        virtual int GetBytesProcessed() const override;
        virtual int GetBitsProcessed() const override;
        virtual int GetBitsRemaining() const override;
        virtual int GetBytesRemaining() const override;
        virtual int GetAlignBits() const override;
        virtual int GetTotalBits() const override;
        virtual int GetTotalBytes() const override;

    private:

        virtual bool SerializeInteger(int32_t& InValue, int32_t InMin, int32_t InMax);
        virtual bool SerializeBits(uint32_t& InValue, int InBitsCount);
        virtual bool SerializeBytes(const uint8_t* InData, int InBytesCount);
        virtual bool SerializeAlign();
        virtual bool SerializeCheck(const char* string);
        virtual int GetError() const;

        int Error;
    };
}

