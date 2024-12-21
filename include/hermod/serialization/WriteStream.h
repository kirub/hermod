#pragma once
#include "Stream.h"
#include "BitWriter.h"

namespace serialization
{
    class HERMOD_API WriteStream :
        public IStream
    {
    public:

        WriteStream(unsigned char* InBuffer, int InSizeInBytes);
        virtual void Reset();
        virtual const uint8_t* GetData();
        virtual int GetDataSize() const;

    private:

        virtual bool SerializeInteger(int32_t& InValue, int32_t InMin, int32_t InMax);
        virtual bool SerializeBits(uint32_t& InValue, int InBitsCount);
        virtual bool SerializeBytes(const uint8_t* InData, int InBytesCount);
        virtual bool SerializeAlign();
        virtual bool SerializeCheck(const char* string);

        int GetAlignBits() const;
        void Flush();

        int GetBytesProcessed() const;
        int GetBitsProcessed() const;
        int GetBitsRemaining() const;
        int GetTotalBits() const;
        int GetTotalBytes() const;

        virtual int GetError() const;

        int Error;
        BitWriter Writer;
    };
}

