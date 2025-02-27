#pragma once

#include <hermod/utilities/TypeTraits.h>
#include <hermod/utilities/Utils.h>
#include <hermod/serialization/NetTypeTraits.h>
#include <hermod/serialization/NetIdMapping.h>

#include <cassert>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <string>

namespace serialization
{
    class NetObjectSerializationCache;

	class HERMOD_API IStream
	{

	public:
        using Deleter = void(*)(unsigned char*);

        enum EOperationType
        {
            Reading,
            Writing
        };

        IStream(EOperationType InOpType);
        virtual void Reset() = 0;

        operator bool() const
        {
            return IsValid();
        }

        virtual bool IsValid() const { return true; }
        virtual int GetError() const = 0;
        virtual const uint8_t* GetData();
        virtual int GetDataSize() const;
        virtual bool WouldOverflow(int bits) const = 0;
        virtual void AdjustSize(int InNumBytes);

        virtual int GetAlignBits() const;
        virtual int GetBytesProcessed() const;
        virtual int GetBitsProcessed() const;
        virtual int GetBitsRemaining() const;
        virtual int GetBytesRemaining() const;
        virtual int GetTotalBits() const;
        virtual int GetTotalBytes() const;

        bool IsWriting() const;
        bool IsReading() const;
        std::string Operation() const;

        bool InsertObjectBuffer(NetObjectDataPtr NetObjectData);
        bool Align(uint32_t AlignToBits = 8);
        bool Guard(const char* InString);
        virtual bool Flush() = 0;


        template < typename ValueType >
        bool Serialize(ValueType& InOutValue, const serialization::NetPropertySettings<ValueType>& Properties = serialization::NetPropertySettings<ValueType>())
        {
            return false;
        }

        template_with_concept_define(Boolean, Boolean<, T)
		//template <typename T, typename enable_if<Boolean<T>::Value>::Type>
        bool Serialize(T& InOutValue, const serialization::NetPropertySettings<T>& Properties = serialization::NetPropertySettings<T>())
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

		template_with_concept_define(Pointer, Pointer<, T)
        bool Serialize(T& InOutValue, const serialization::NetPropertySettings<T>& Properties = serialization::NetPropertySettings<T>())
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
        bool Serialize(uint64_t& InOutValue, const serialization::NetPropertySettings<uint64_t>& Properties)
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

		template_with_concept_define(Signed, Signed<, T)
        bool Serialize(T& InOutValue, const serialization::NetPropertySettings<T>& Properties = serialization::NetPropertySettings<T>())
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
        bool Serialize(int64_t& InOutValue, const serialization::NetPropertySettings<int64_t>& Properties)
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

		template_with_concept_define(Unsigned, Unsigned<, T)
        bool Serialize(T& InOutValue, const serialization::NetPropertySettings<T>& Properties = serialization::NetPropertySettings<T>())
        {
            return serialize_bits(InOutValue, utils::bits_required(0, Properties.Max));
        }

		template_with_concept_define(Float, Float<, T)
        bool Serialize(T& InOutValue, const serialization::NetPropertySettings<T>& Properties = serialization::NetPropertySettings<T>())
        {
            bool ReturnValue = true;
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
                ReturnValue = Serialize(IntegerValue, serialization::NetPropertySettings<uint64_t>(MaxIntegerValue));
            }
            else
            {
                uint32_t IntegerValueAsUint32 = (uint32_t)IntegerValue;
                ReturnValue = SerializeBits(IntegerValueAsUint32, BitsRequired);
                IntegerValue = (uint64_t)IntegerValueAsUint32;
            }

            if (ReturnValue && IsReading())
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
            return ReturnValue;
        }
        template <>
        bool Serialize<std::string>(std::string& InOutValue, const serialization::NetPropertySettings<std::string>& Properties)
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

		template_with_concept_define(StringBuffer, StringBuffer<, T)
        bool Serialize(T& InOutValue, const serialization::NetPropertySettings<T>& Properties = serialization::NetPropertySettings<T>())
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

		template_with_concept_define(Buffer, Buffer<, T)
        bool Serialize(T& InOutValue, const serialization::NetPropertySettings<T>& Properties = serialization::NetPropertySettings<T>())
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

		template_with_concept_define(Enumeration, Enumeration<, T)
        bool Serialize(T& InOutValue, const serialization::NetPropertySettings<T>& Properties = serialization::NetPropertySettings<T>())
        {
            static_assert(std::is_standard_layout_v<std::underlying_type<T>>, "Enumeration underlying type must be pod");
            std::underlying_type_t<T> PODTypeValue;
            if (IsWriting())
            {
                PODTypeValue = to_underlying(InOutValue);
            }
            Serialize(PODTypeValue, { 0, Properties.Max });
            if (IsReading())
            {
                InOutValue = static_cast<T>(PODTypeValue);
            }

            return true;
        }

	private:

        template < typename InputType>
        bool serialize_bits(InputType& InOutValue, int Bits)
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
        }

		virtual bool SerializeBits(uint32_t& value, int bits) = 0;
		virtual bool SerializeBytes(const uint8_t* data, int bytes) = 0;
		virtual bool SerializeInteger(int32_t& value, int32_t min, int32_t max) = 0;
        virtual bool SerializeAlign(uint32_t AlignToBits = 8) = 0;
        virtual bool SerializeCheck(const char* InString) = 0;

        EOperationType OpType;
	};
}
