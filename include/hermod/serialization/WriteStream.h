#pragma once
#include "Stream.h"
#include "BitWriter.h"

namespace serialization
{
    class HERMOD_API WriteStream :
        public IStream
    {
    public:

        WriteStream(int InSizeInBytes);
        WriteStream(unsigned char* InBuffer, int InSizeInBytes);

        virtual ~WriteStream();
        virtual void Reset();
        virtual const uint8_t* GetData();
        virtual int GetDataSize() const;

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

        void Flush();


        virtual int GetError() const;

        int Error;
        BitWriter Writer;
        bool OwnsBuffer;
    };
}

