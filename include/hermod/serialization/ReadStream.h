#pragma once

#include "Stream.h"
#include "BitReader.h"

namespace serialization
{
    class HERMOD_API ReadStream :
        public IStream
    {
    public:
        ReadStream(unsigned char* InBuffer, int InSizeInBytes);

        virtual void Reset();
        virtual const uint8_t* GetData();
        virtual int GetDataSize() const;
    private:

        virtual bool SerializeInteger(int32_t& OutValue, int32_t InMin, int32_t InMax);
        virtual bool SerializeBits(uint32_t& OutVvalue, int InBitsCount);
        virtual bool SerializeBytes(const uint8_t* OutData, int InBytesCount);
        virtual bool SerializeAlign();
        virtual bool SerializeCheck(const char* InString);


        int GetAlignBits() const;
        int GetBitsProcessed() const;
        int GetBitsRemaining() const;
        int GetBytesProcessed() const;
        int GetBytesRead() const;

        virtual int GetError() const;

    private:

        int Error;
        int BitsRead;
        BitReader Reader;
    };
}
