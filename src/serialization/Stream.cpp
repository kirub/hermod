#include <hermod/serialization/Stream.h>
#include <hermod/serialization/NetIdMapping.h>
#include <hermod/serialization/NetObjectSerializationCache.h>
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

    bool IStream::InsertObjectBuffer(NetObjectDataPtr NetObjectData)
    {
        if (NetObjectData && *NetObjectData)
        {
            return SerializeBytes((uint8_t*)NetObjectData->Buffer, (int)NetObjectData->BufferSize);
        }

        return false;
    }

    bool IStream::IsWriting() const { return OpType == Writing; }

    bool IStream::IsReading() const { return OpType == Reading; }

    std::string IStream::Operation() const { return IsReading() ? "read" : "write"; }

    bool IStream::Guard(const char* string)
    {
        return SerializeCheck(string);
    }
    bool IStream::Align(uint32_t AlignToBits /*= 8*/)
    {
        return SerializeAlign(AlignToBits);
    }

    const uint8_t* IStream::GetData() { return nullptr; };
    int IStream::GetDataSize() const { return 0; }
    void IStream::AdjustSize(int InNumBytes) {}

    int IStream::GetAlignBits() const { return 0; }
    int IStream::GetBytesProcessed() const { return 0; }
    int IStream::GetBitsProcessed() const { return 0; }
    int IStream::GetBitsRemaining() const { return 0; }
    int IStream::GetBytesRemaining() const { return 0; }
    int IStream::GetTotalBits() const { return 0; }
    int IStream::GetTotalBytes() const { return 0; }

    /*
    template < typename ValueType >
    bool IStream::Serialize(ValueType& InOutValue, const serialization::NetPropertySettings<ValueType>& Properties)
    {
        return false;
    }

    template <serialization::Boolean T>
    bool IStream::Serialize(T& InOutValue, const serialization::NetPropertySettings<T>& Properties)
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

    template <Pointer T>
    bool IStream::Serialize(T& InOutValue, const serialization::NetPropertySettings<T>& Properties)
    {
        NetObjectId Max = InvalidNetObjectId & PTR_TO_ID_MASK;
        uint32_t hi, lo;
        if (IsWriting())
        {
            NetObjectId OutValue = serialization::NetIdMapping::Get().GetOrAssignedNetId(InOutValue);
            lo = OutValue & 0xFFFFFFFF;
            hi = OutValue >> 32;
        }
        Serialize(lo, serialization::NetPropertySettings<uint32_t>(Max & 0xFFFFFFFF));
        Serialize(hi, serialization::NetPropertySettings<uint32_t>(Max >> 32));
        if (IsReading())
        {
            NetObjectId InValue = (uint64_t(hi) << 32) | lo;
            InOutValue = serialization::NetIdMapping::Get().GetObjectFromNetId(InValue);
        }
        return true;
    }
    template <>
    bool IStream::Serialize(uint64_t& InOutValue, const serialization::NetPropertySettings<uint64_t>& Properties)
    {
        uint32_t hi, lo;
        if (IsWriting())
        {
            lo = InOutValue & 0xFFFFFFFF;
            hi = InOutValue >> 32;
        }
        Serialize(lo, serialization::NetPropertySettings<uint32_t>(Properties.Max & 0xFFFFFFFF));
        Serialize(hi, serialization::NetPropertySettings<uint32_t>(Properties.Max >> 32));
        if (IsReading())
        {
            InOutValue = (uint64_t(hi) << 32) | lo;
        }
        return true;
    }
    template <serialization::Signed T>
    bool IStream::Serialize(T& InOutValue, const serialization::NetPropertySettings<T>& Properties)
    {
        assert(Properties.Min < Properties.Max);
        int32_t int32_value = 0;
        if (IsWriting())
        {
            assert(int64_t(InOutValue) >= int64_t(Properties.Min));
            assert(int64_t(InOutValue) <= int64_t(Properties.Max));
            int32_value = (int32_t)InOutValue;
        }
        if (!SerializeInteger(int32_value, Properties.Min, Properties.Max))
        {
            return false;
        }
        if (IsReading())
        {
            InOutValue = int32_value;
            if (InOutValue < Properties.Min || InOutValue > Properties.Max)
                return false;
        }

        return true;
    }
    template <>
    bool IStream::Serialize(int64_t& InOutValue, const serialization::NetPropertySettings<int64_t>& Properties)
    {
        uint64_t ValueAsUnsigned = 0;
        if (IsWriting())
        {
            ValueAsUnsigned = static_cast<uint64_t>(InOutValue);
        }
        serialization::NetPropertySettings<uint64_t> PropertiesAsUnsigned(Properties.Max - Properties.Min);
        Serialize(ValueAsUnsigned, PropertiesAsUnsigned);
        if (IsReading())
        {
            InOutValue = static_cast<int64_t>(ValueAsUnsigned);
        }
        return true;
    }
    template <serialization::Unsigned T>
    bool IStream::Serialize(T& InOutValue, const serialization::NetPropertySettings<T>& Properties)
    {
        return serialize_bits(InOutValue, utils::bits_required(0, Properties.Max));
    }

    template <std::floating_point T>
    bool IStream::Serialize(T& InOutValue, const serialization::NetPropertySettings<T>& Properties)
    {
        uint64_t IntegerValue = 0;
        uint8_t BitsRequired = sizeof(T) * 8;
        uint64_t MaxIntegerValue;
        T Delta = 0.0f;
        if (!Properties.IsDefault())
        {
            Delta = (Properties.Max - Properties.Min);
            const T Values = Delta / Properties.Precision;
            MaxIntegerValue = (uint64_t)ceil(Values);
            BitsRequired = utils::bits_required(0, (uint32_t)MaxIntegerValue);
            if (IsWriting())
            {
                T NormalizedValue = std::clamp((InOutValue - Properties.Min) / Delta, T(0.0), T(1.0f));
                IntegerValue = (uint64_t)floor(NormalizedValue * MaxIntegerValue + 0.5f);
            }
        }
        else
        {
            if (IsWriting())
            {
                union DoubleInt
                {
                    T float_value;
                    uint64_t int_value;
                } Temp;
                Temp.float_value = InOutValue;
                IntegerValue = Temp.int_value;
            }
            MaxIntegerValue = static_cast<uint64_t>(-1);
        }

        if (BitsRequired > 32)
        {
            Serialize(IntegerValue, serialization::NetPropertySettings<uint64_t>(MaxIntegerValue));
        }
        else
        {
            uint32_t IntegerValueAsUint32 = (uint32_t)IntegerValue;
            SerializeBits(IntegerValueAsUint32, BitsRequired);
            IntegerValue = (uint64_t)IntegerValueAsUint32;
        }

        if (IsReading())
        {
            if (!Properties.IsDefault())
            {
                const T NormalizedValue = IntegerValue / float(MaxIntegerValue);
                InOutValue = NormalizedValue * Delta + Properties.Min;
            }
            else
            {
                union DoubleInt
                {
                    T float_value;
                    uint64_t int_value;
                } Temp;
                Temp.int_value = IntegerValue;
                InOutValue = Temp.float_value;
            }
        }
        return true;
    }
    template <>
    bool IStream::Serialize<std::string>(std::string& InOutValue, const serialization::NetPropertySettings<std::string>& Properties)
    {
        if (IsWriting())
        {
            assert(Properties.Length < uint16_t(std::numeric_limits<uint16_t>::max() / 8 - 1));
        }
        uint32_t Length = (uint32_t)(Properties.Length == 0 ? InOutValue.length() + 1 : (uint32_t)Properties.Length);
        SerializeBits(Length, 16);
        if (IsReading())
        {
            InOutValue.reserve(Length);
            InOutValue.resize(Length);
        }
        SerializeBytes((uint8_t*)&InOutValue[0], Length);
        return true;
    }
    template <serialization::StringBuffer T>
    bool IStream::Serialize(T& InOutValue, const serialization::NetPropertySettings<T>& Properties)
    {
        if (IsWriting())
        {
            assert(Properties.Length < std::size_t((std::numeric_limits<uint32_t>::max() / 8) - 1));
        }
        uint32_t Length = (uint32_t)Properties.Length;
        SerializeBits(Length, 16);
        SerializeBytes((uint8_t*)InOutValue, Length);
        if (IsReading())
        {
            InOutValue[Length] = '\0';
        }
        return true;
    }
    template <serialization::Buffer T>
    bool IStream::Serialize(T& InOutValue, const serialization::NetPropertySettings<T>& Properties)
    {
        if (IsWriting())
        {
            assert(Properties.Length < std::size_t((std::numeric_limits<uint32_t>::max() / 8) - 1));
        }
        uint32_t Length = (uint32_t)Properties.Length;
        SerializeBits(Length, 16);
        SerializeBytes((uint8_t*)InOutValue, Length);
        return true;
    }
    template < serialization::Enumeration T >
    bool IStream::Serialize(T& InOutValue, const serialization::NetPropertySettings<T>& Properties)
    {
        static_assert(std::is_standard_layout_v<std::underlying_type<T>>, "Enumeration underlying type must be pod");
        std::underlying_type_t<T> PODTypeValue;
        if (IsWriting())
        {
            PODTypeValue = std::to_underlying(InOutValue);
        }
        Serialize(PODTypeValue, { 0, Properties.Max });
        if (IsReading())
        {
            InOutValue = static_cast<T>(PODTypeValue);
        }

        return true;
    }

    template < typename InputType>
    bool IStream::serialize_bits(InputType& InOutValue, int Bits)
    {
        assert(Bits > 0);
        assert(Bits <= 32);
        uint32_t uint32_value = 0;
        if (IsWriting())
        {
            uint32_value = static_cast<uint32_t>(InOutValue);
        }
        if (!SerializeBits(uint32_value, Bits))
        {
            return false;
        }
        if (IsReading())
        {
            InOutValue = static_cast<InputType>(uint32_value);
        }
        return true;
    }*/


    /*template bool IStream::Serialize<bool>(bool&, const serialization::NetPropertySettings<bool>&);
    template bool IStream::Serialize< proto::INetObject* >(proto::INetObject*&, const serialization::NetPropertySettings<proto::INetObject*>&);
    template bool IStream::Serialize(int8_t&, const serialization::NetPropertySettings<int8_t>&);
    template bool IStream::Serialize(int16_t&, const serialization::NetPropertySettings<int16_t>&);
    template bool IStream::Serialize(int32_t&, const serialization::NetPropertySettings<int32_t>&);
    template bool IStream::Serialize<int64_t>(int64_t&, const serialization::NetPropertySettings<int64_t>&);
    template bool IStream::Serialize<uint8_t>(uint8_t&, const serialization::NetPropertySettings<uint8_t>&);
    template bool IStream::Serialize<uint16_t>(uint16_t&, const serialization::NetPropertySettings<uint16_t>&);
    template bool IStream::Serialize<uint32_t>(uint32_t&, const serialization::NetPropertySettings<uint32_t>&);
    template bool IStream::Serialize<uint64_t>(uint64_t&, const serialization::NetPropertySettings<uint64_t>&);
    template bool IStream::Serialize<float>(float&, const serialization::NetPropertySettings<float>&);
    template bool IStream::Serialize<double>(double&, const serialization::NetPropertySettings<double>&);
    template bool IStream::Serialize<std::string>(std::string&, const serialization::NetPropertySettings<std::string>&);
    template bool IStream::Serialize<char*>(char*&, const serialization::NetPropertySettings<char*>&);
    template bool IStream::Serialize<unsigned char*>(unsigned char*&, const serialization::NetPropertySettings<unsigned char*>&);
    template bool IStream::Serialize<uint8_t*>(uint8_t*&, const serialization::NetPropertySettings<uint8_t*>&);

    template bool IStream::serialize_bits<bool>(bool&, int);
    template bool IStream::serialize_bits<uint8_t>(uint8_t&, int);
    template bool IStream::serialize_bits<uint16_t>(uint16_t&, int);
    template bool IStream::serialize_bits<uint32_t>(uint32_t&, int);*/
}