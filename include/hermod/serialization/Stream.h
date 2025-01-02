#pragma once
#include <hermod/serialization/NetTypeTraits.h>
#include <hermod/utilities/Utils.h>

#include <cassert>
#include <cstdint>
#include <concepts>
#include <bit>
#include <limits>
#include <algorithm>
#include <string>

namespace serialization
{
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

        virtual int GetError() const = 0;
        virtual const uint8_t* GetData() { return nullptr; };
        virtual int GetDataSize() const { return 0; }
        virtual bool WouldOverflow(int bits) const = 0;

        virtual int GetAlignBits() const { return 0; }
        virtual int GetBytesProcessed() const { return 0; }
        virtual int GetBitsProcessed() const { return 0; }
        virtual int GetBitsRemaining() const { return 0; }
        virtual int GetBytesRemaining() const { return 0; }
        virtual int GetTotalBits() const { return 0; }
        virtual int GetTotalBytes() const { return 0; }

        bool IsWriting() const;
        bool IsReading() const;
        std::string Operation() const;

        virtual bool Align();
        virtual bool Guard(const char* InString);
        virtual void EndWrite() {}

        template < typename ValueType >
        bool Serialize(ValueType& InOutValue, const serialization::NetPropertySettings<ValueType>& Properties = serialization::NetPropertySettings<ValueType>())
        {
            return false;
        }

        template <serialization::Boolean T>
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
        template <serialization::Signed T>
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
        template <serialization::Unsigned T>
        bool Serialize(T& InOutValue, const serialization::NetPropertySettings<T>& Properties = serialization::NetPropertySettings<T>())
        {
            return serialize_bits(InOutValue, utils::bits_required(0, Properties.Max));
        }

        template <std::floating_point T>
        bool Serialize(T& InOutValue, const serialization::NetPropertySettings<T>& Properties = serialization::NetPropertySettings<T>())
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
        bool Serialize<std::string>(std::string& InOutValue, const serialization::NetPropertySettings<std::string>& Properties)
        {
            if (IsWriting())
            {
                assert(Properties.Length < uint16_t(std::numeric_limits<uint16_t>::max()/8 - 1));
            }
            uint32_t Length = (uint32_t)Properties.Length;
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
        bool Serialize(T& InOutValue, const serialization::NetPropertySettings<T>& Properties = serialization::NetPropertySettings<T>())
        {
            if (IsWriting())
            {
                assert(Properties.Length < std::size_t((std::numeric_limits<uint32_t>::max()/8) - 1));
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
        template < serialization::Enumeration T >
        bool Serialize(T& InOutValue, const serialization::NetPropertySettings<T>& Properties = serialization::NetPropertySettings<T>())
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
        virtual bool SerializeAlign() = 0;
        virtual bool SerializeCheck(const char* InString) = 0;

        EOperationType OpType;
	};
}