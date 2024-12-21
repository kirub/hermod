#include <hermod/serialization/Stream.h>
#include <string.h>

namespace serialization
{

    IStream::IStream(EOperationType InOpType)
        : OpType(InOpType)
    {
    }
    void Reset()
    {

    }

    bool IStream::IsWriting() const { return OpType == Writing; }

    bool IStream::IsReading() const { return OpType == Reading; }

    std::string IStream::Operation() const { return IsReading() ? "read" : "write"; }
    /*
    bool IStream::Serialize(float& InOutValue)
    {
        uint32_t int_value = 0;
        if (IsWriting())
        {
            memcpy(&int_value, &InOutValue, 4);
        }

        bool result = SerializeBits(int_value, 32);

        if (IsReading())
        {
            memcpy(&InOutValue, &int_value, 4);
        }

        return result;
    }
    bool IStream::Serialize(int8_t& InOutValue, int8_t InMin, int8_t InMax)
    {
        return Serialize((int32_t&)InOutValue, (int32_t)InMin, (int32_t)InMax);

    }
    bool IStream::Serialize(int16_t& InOutValue, int16_t InMin, int16_t InMax)
    {
        return Serialize((int16_t&)InOutValue, (int16_t)InMin, (int16_t)InMax);
    }

    bool IStream::Serialize(int32_t& InOutValue, int32_t InMin, int32_t InMax)
    {
        assert(InMin < InMax);
        int32_t int32_value = 0;
        if (IsWriting())
        {
            assert(int64_t(InOutValue) >= int64_t(InMin));
            assert(int64_t(InOutValue) <= int64_t(InMax));
            int32_value = (int32_t)InOutValue;
        }
        if (!SerializeInt(int32_value, InMin, InMax))
        {
            return false;
        }
        if (IsReading())
        {
            InOutValue = int32_value;
            if (InOutValue < InMin || InOutValue > InMax)
                return false;
        }

        return true;
    }

    bool IStream::Serialize(bool& InOutValue)
    {
        uint32_t uint32_bool_value;
        if (IsWriting())
        {
            uint32_bool_value = InOutValue ? 1 : 0;
        }
        if (!serialize_bits(uint32_bool_value, 1))
        {
            return false;
        }
        if (IsReading())
        {
            InOutValue = uint32_bool_value ? true : false;
        }

        return true;
    }


    bool IStream::Serialize(uint8_t& InOutValue)
    {
        return serialize_bits(InOutValue, 8);

    }

    bool IStream::Serialize(uint16_t_t& InOutValue)
    {
        return serialize_bits(InOutValue, 16);
    }

    bool IStream::Serialize(uint32_t& InOutValue)
    {
        return serialize_bits(InOutValue, 32);
    }

    bool IStream::Serialize(uint64_t& InOutValue)
    {
        uint32_t hi, lo;
        if (IsWriting())
        {
            lo = InOutValue & 0xFFFFFFFF;
            hi = InOutValue >> 32;
        }
        serialize_bits(lo, 32);
        serialize_bits(hi, 32);
        if (IsReading())
        {
            InOutValue = (uint64_t(hi) << 32) | lo;
        }
        return true;
    }

    bool IStream::Serialize(double& InOutValue)
    {
        union DoubleInt
        {
            double double_value;
            uint64_t int_value;
        };

        DoubleInt tmp;
        if (IsWriting())
        {
            tmp.double_value = InOutValue;
        }

        Serialize(tmp.int_value);

        if (IsReading())
        {
            InOutValue = tmp.double_value;
        }

        return true;
    }

    bool IStream::Serialize(uint8_t* data, int bytes)
    {
        return SerializeBytes(data, bytes);
    }

    bool IStream::Serialize(char* string, int buffer_size)
    {
        int length;
        if (IsWriting())
        {
            length = (int)strlen(string);
            assert(length < buffer_size - 1);
        }
        Serialize(length, 0, buffer_size - 1);
        Serialize((uint8_t*)string, length);
        if (IsReading())
        {
            string[length] = '\0';
        }
        return true;
    }*/

    bool IStream::Guard(const char* string)
    {
        if (!SerializeCheck(string))
        {
            return false;
        }

        return true;
    }
    bool IStream::Align()
    {
        if (!SerializeAlign())
        {
            return false;
        }

        return true;
    }
}