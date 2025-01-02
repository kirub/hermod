#pragma once

#undef min
#undef max

#include <hermod/platform/Platform.h>
#include <hermod/utilities/Utils.h>

#include <type_traits>
#include <array>
#include <string>
#include <concepts>

namespace serialization
{
	template <typename T>
	struct array_size : std::extent<T> {};
	template <typename T, size_t N>
	struct array_size<std::array<T, N> > : std::tuple_size<std::array<T, N> > {};
	template<typename T>
	struct is_string
		: public std::disjunction<
		std::is_same<const char*, std::decay_t<T>>,
		std::is_same<char*, std::decay_t<T>>,
		std::is_same<std::string, std::decay_t<T>>
		> {
	};

	template<typename T>
	struct is_raw_buffer
		: public std::disjunction<
		std::is_same<const unsigned char*, std::decay_t<T>>,
		std::is_same<unsigned char*, std::decay_t<T>>,
		std::is_same<uint8_t*, std::decay_t<T>>,
		std::is_same<const uint8_t*, std::decay_t<T>>
		> {
	};

	//template <typename NumericType>		concept Numeric = std::is_arithmetic_v<NumericType>;
	template <typename UnsignedType>		concept Unsigned = std::unsigned_integral<UnsignedType> && !std::is_same_v<UnsignedType, uint64_t>;
	template <typename SignedType>			concept Signed = std::signed_integral<SignedType> && !std::is_same_v<SignedType, int64_t>;
	template <typename ArrayType>			concept Array = std::is_bounded_array_v<ArrayType>;
	template <typename EnumType>			concept Enumeration = std::is_enum_v<EnumType>;
	template <typename PointerType>			concept Pointer = std::is_pointer_v<PointerType>;
	template <typename BufferType>			concept Buffer = is_raw_buffer<BufferType>::value && !is_string<BufferType>::value && !Enumeration<BufferType>;
	template <typename StringType>			concept String = is_string<StringType>::value && !Buffer< StringType>;
	template <typename StringBufferType>	concept StringBuffer = String<StringBufferType> && !std::is_same_v<StringBufferType, std::string>;
	template <typename BoolType>			concept Boolean = std::_Is_character_or_byte_or_bool<BoolType>::value && !std::integral<BoolType>;

	template < typename PropertyType >
		requires std::unsigned_integral<PropertyType> || std::signed_integral<PropertyType> 
	|| std::floating_point<PropertyType>
	|| Array<PropertyType> || Pointer<PropertyType> 
	|| String<PropertyType> || Buffer<PropertyType>
	|| Boolean<PropertyType> || Enumeration<PropertyType>
	struct NetPropertySettings
	{
	};

	template <std::signed_integral T>
	struct NetPropertySettings<T>
	{
		T Min;
		T Max;

		NetPropertySettings()
			: Min(std::numeric_limits<T>::min())
			, Max(std::numeric_limits<T>::max())
		{
		}
		NetPropertySettings(const T InMin, const T InMax)
			: Min(InMin)
			, Max(InMax)
		{
		}
	};	

	template <std::unsigned_integral T>
	struct NetPropertySettings<T>
	{
		T Max;

		NetPropertySettings()
			: Max(std::numeric_limits<T>::max())
		{
		}
		NetPropertySettings(const T InMax)
			: Max(InMax)
		{
		}
	};

	template <Array T>
	struct NetPropertySettings<T>
	{
		T Max[std::rank_v<T>];

		NetPropertySettings(T InArray)
		{
			for (int Idx : std::rank_v<T>)
			{
				Max[Idx] = std::extent_v<T, Idx>;
			}
		}

	};
	template <Enumeration T>
	struct NetPropertySettings<T>
	{
		std::underlying_type_t<T> Max;

		NetPropertySettings()
		{
			Max = std::to_underlying(T::Count);
		}

		NetPropertySettings(const T InCount)
			: Max(std::to_underlying(InCount))
		{
		}
	};
	template <std::floating_point T>
	struct NetPropertySettings<T>
	{

		T Min = std::numeric_limits<T>::min();
		T Max = std::numeric_limits<T>::max();
		uint32_t Precision = std::numeric_limits<T>::digits10;

		NetPropertySettings()
		{			
		}

		bool IsDefault() const {
			return Min == std::numeric_limits<T>::min() || std::numeric_limits<T>::max();
		}

		NetPropertySettings(const T InMin, const T InMax)
			: Min(InMin)
			, Max(InMax)
		{
		}

		NetPropertySettings(const T InMin, const T InMax, uint32_t InPrecision)
			: Min(InMin)
			, Max(InMax)
			, Precision(InPrecision)
		{
		}
	};


	template <String T>
	struct NetPropertySettings<T>
	{
		std::size_t Length = 0;

		NetPropertySettings()
			: Length(0)
		{
		}
		NetPropertySettings(const std::size_t InLength)
			: Length(InLength+1)
		{
		}
		NetPropertySettings(const char* InString)
			: Length(std::char_traits<char>::length(InString)+1)
		{
		}
		NetPropertySettings(const std::string& InString)
			: Length(InString.length()+1)
		{
		}
	};

	template <Buffer T>
	struct NetPropertySettings<T>
	{
		std::size_t Length = 0;

		NetPropertySettings(const std::size_t InLength)
			: Length(InLength)
		{
		}
	};

	/*template struct HERMOD_API NetPropertySettings<uint8_t>;
	template struct HERMOD_API NetPropertySettings<uint16_t>;
	template struct HERMOD_API NetPropertySettings<uint32_t>;
	template struct HERMOD_API NetPropertySettings<uint64_t>;
			 
	template struct HERMOD_API NetPropertySettings<int8_t>;
	template struct HERMOD_API NetPropertySettings<int16_t>;
	template struct HERMOD_API NetPropertySettings<int32_t>;
	template struct HERMOD_API NetPropertySettings<int64_t>;
			 
	template struct HERMOD_API NetPropertySettings<std::string>;
	template struct HERMOD_API NetPropertySettings<const char*>;
	template struct HERMOD_API NetPropertySettings<const unsigned char*>;
	template struct HERMOD_API NetPropertySettings<char*>;
	template struct HERMOD_API NetPropertySettings<unsigned char*>;
			 
	template struct HERMOD_API NetPropertySettings<float>;
	template struct HERMOD_API NetPropertySettings<double>;*/
}