#pragma once

#undef min
#undef max

#include <hermod/platform/Platform.h>
#include <hermod/utilities/Utils.h>

#include <hermod/utilities/TypeTraits.h>
#include <string>

namespace proto
{
	class INetObject;
}

namespace serialization
{
	template_with_concept_base(NetPropertySettings, PropertyType, 
		std::unsigned_integral<PropertyType> || std::signed_integral<PropertyType> || std::floating_point<PropertyType> || ArrayType<PropertyType> ||
		Pointer<PropertyType> || StringType<PropertyType> || Buffer<PropertyType> || Boolean<PropertyType> || Enumeration<PropertyType>)
	{
	};

	template_with_concept_declare(std::signed_integral, is_signed_integral_type<, NetPropertySettings, T)
	{
		T Min;
		T Max;

		NetPropertySettings() :
				Min(std::numeric_limits<T>::min()), Max(std::numeric_limits<T>::max()) {
		}
		NetPropertySettings(const T InMin, const T InMax) :
				Min(InMin), Max(InMax) {
		}
	};

	template_with_concept_declare(std::unsigned_integral, is_unsigned_integral_type < , NetPropertySettings, T)
	{
		T Max;

		NetPropertySettings() :
				Max(std::numeric_limits<T>::max()) {
		}
		NetPropertySettings(const T InMax) :
				Max(InMax) {
		}
	};

	template_with_concept_declare(ArrayType, ArrayType<, NetPropertySettings, T)
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

	template_with_concept_declare(Enumeration, Enumeration<, NetPropertySettings, T)
	{
		std::underlying_type_t<T> Max;

		NetPropertySettings()
		{
			Max = to_underlying(T::Count);
		}

		NetPropertySettings(const T InCount)
			: Max(to_underlying(InCount))
		{
		}
	};

	template_with_concept_declare(Float, Float<, NetPropertySettings, T)
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

	template_with_concept_declare(StringType, StringType<, NetPropertySettings, T)
	{
		std::size_t Length = 0;

		NetPropertySettings()
			: Length(0)
		{
		}
		NetPropertySettings(const std::size_t InLength)
			: Length(InLength)
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

	template_with_concept_declare(Buffer, Buffer<, NetPropertySettings, T)
	{
		std::size_t Length = 0;

		NetPropertySettings(const std::size_t InLength)
			: Length(InLength)
		{
		}
	};

}
