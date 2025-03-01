#pragma once
#include "Stream.h"
#include "BitWriter.h"

namespace serialization
{
    class HERMOD_API WriteStream :
        public IStream
    {
    public:

        WriteStream();
        WriteStream(int InSizeInBytes);
        WriteStream(unsigned char* InBuffer, int InSizeInBytes);
        WriteStream(unsigned char* InBuffer, int InSizeInBytes, Deleter InDeleter);
        WriteStream(const WriteStream& Rhs);
        WriteStream& operator=(const WriteStream& Rhs);

        virtual bool IsValid() const;

        virtual ~WriteStream();
        void Clear();
        WriteStream Shift(std::size_t Offset);
        virtual void Reset();
        virtual const uint8_t* GetData();
        virtual int GetDataSize() const;
        virtual bool Flush() override;

        virtual bool WouldOverflow(int bytes) const override;

        virtual int GetBytesProcessed() const override;
        virtual int GetBitsProcessed() const override;
        virtual int GetBitsRemaining() const override;
        virtual int GetBytesRemaining() const override;
        virtual int GetAlignBits() const override;
        virtual int GetTotalBits() const override;
        virtual int GetTotalBytes() const override;

    private:

        virtual bool SerializeInteger(int32_t& InValue, int32_t InMin, int32_t InMax) override;
        virtual bool SerializeBits(uint32_t& InValue, int InBitsCount) override;
        virtual bool SerializeBytes(const uint8_t* InData, int InBytesCount) override;
        virtual bool SerializeAlign(uint32_t AlignToBits = 8) override;
        virtual bool SerializeCheck(const char* string) override;


        virtual int GetError() const;

        int Error;
        unsigned char* Data;
        BitWriter Writer;
        Deleter DeleterFunc;
    };
}

