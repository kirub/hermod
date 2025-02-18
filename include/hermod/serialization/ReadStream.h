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

        ReadStream(int InSizeInBytes);
        ReadStream(unsigned char* InBuffer, int InSizeInBytes);
        ReadStream(unsigned char* InBuffer, int InSizeInBytes, Deleter InDeleter);
        virtual ~ReadStream();

        ReadStream Shift(std::size_t Offset);

        virtual void Reset() override;
        virtual void AdjustSize(int InNumBytes) override;
        virtual const uint8_t* GetData() override;
        virtual int GetDataSize() const override;
        virtual bool Flush() override;

        virtual bool WouldOverflow(int bits) const override;

        virtual int GetBytesProcessed() const override;
        virtual int GetBitsProcessed() const override;
        virtual int GetBitsRemaining() const override;
        virtual int GetBytesRemaining() const override;
        virtual int GetAlignBits() const override;
    private:

        virtual bool SerializeInteger(int32_t& OutValue, int32_t InMin, int32_t InMax) override;
        virtual bool SerializeBits(uint32_t& OutVvalue, int InBitsCount) override;
        virtual bool SerializeBytes(const uint8_t* OutData, int InBytesCount) override;
        virtual bool SerializeAlign(uint32_t AlignToBits = 8) override;
        virtual bool SerializeCheck(const char* InString) override;


        int GetBytesRead() const;
        virtual int GetError() const;

    private:

        int Error;
        int BitsRead;
        unsigned char* Data;
        BitReader Reader;
        Deleter DeleterFunc;
    };
}
