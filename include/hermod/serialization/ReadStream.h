#pragma once

#include "Stream.h"
#include "BitReader.h"
#include <functional>
#include <memory>

namespace serialization
{
    class HERMOD_API ReadStream :
        public IStream
    {
    public:
        using Deleter = void(*)(unsigned char*);

        ReadStream(int InSizeInBytes);
        ReadStream(unsigned char* InBuffer, int InSizeInBytes);
        ReadStream(unsigned char* InBuffer, int InSizeInBytes, Deleter InDeleter);
        virtual ~ReadStream();

        virtual void Reset();
        virtual const uint8_t* GetData();
        virtual int GetDataSize() const;

        virtual int GetBytesProcessed() const override;
        virtual int GetBitsProcessed() const override;
        virtual int GetBitsRemaining() const override;
        virtual int GetBytesRemaining() const override;
        virtual int GetAlignBits() const override;
    private:

        virtual bool SerializeInteger(int32_t& OutValue, int32_t InMin, int32_t InMax);
        virtual bool SerializeBits(uint32_t& OutVvalue, int InBitsCount);
        virtual bool SerializeBytes(const uint8_t* OutData, int InBytesCount);
        virtual bool SerializeAlign();
        virtual bool SerializeCheck(const char* InString);


        int GetBytesRead() const;
        virtual int GetError() const;

    private:

        int Error;
        int BitsRead;
        BitReader Reader;
        Deleter DeleterFunc;
    };
}
