#pragma once
#include "Stream.h"
#include "BitWriter.h"

namespace serialization
{
    class HERMOD_API FakeWriteStream :
        public IStream
    {
    public:
        static bool SimulateBitpacker;

        FakeWriteStream(int InSizeInBytes);

        virtual ~FakeWriteStream();
        virtual void Reset();
        virtual const uint8_t* GetData();
        virtual int GetDataSize() const;

        virtual void EndWrite() override;
        virtual bool WouldOverflow(int bytes) const override;

        virtual int GetBytesProcessed() const override;
        virtual int GetBitsProcessed() const override;
        virtual int GetBitsRemaining() const override;
        virtual int GetBytesRemaining() const override;
        virtual int GetAlignBits() const override;
                int GetAlignBits(uint32_t AlignToBits) const;
        virtual int GetTotalBits() const override;
        virtual int GetTotalBytes() const override;

    private:

        virtual bool SerializeInteger(int32_t& InValue, int32_t InMin, int32_t InMax) override;
        virtual bool SerializeBits(uint32_t& InValue, int InBitsCount) override;
        virtual bool SerializeBytes(const uint8_t* InData, int InBytesCount) override;
        virtual bool SerializeAlign(uint32_t AlignToBits = 8) override;
        virtual bool SerializeCheck(const char* string) override;
        virtual int GetError() const;
        void Flush();

        int Error;
        int SizeMax;
        int AccumulatedBits;
        int CurrentSizeInBits;
    };
}

