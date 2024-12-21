#pragma once
#include "NetPropertyInterface.h"

namespace proto
{
	template < typename PropertyType >
	class NetProperty
		: public INetProperty
	{
		//static_assert(std::is_standard_layout<PropertyType>(), "Only handles primitive types");

		PropertyType Value;
		serialization::NetPropertySettings<PropertyType> Properties;


		virtual bool SerializeImpl(serialization::IStream& Stream)
		{
			return Stream.Serialize(Value, Properties);
		}

	public:

		NetProperty(INetObject& Parent, const PropertyType InValue)
			: NetProperty(Parent, InValue, serialization::NetPropertySettings<PropertyType>())
		{
		}
		NetProperty(INetObject& Parent, const PropertyType InValue, const serialization::NetPropertySettings<PropertyType>& InProperties)
			: INetProperty(Parent)
			, Properties(InProperties)
			, Value(InValue)
		{
			operator=(InValue);
		}

		operator const PropertyType& () const
		{
			return Value;
		}

		NetProperty<PropertyType>& operator=(const PropertyType& InProperty)
		{
			if (Value != InProperty)
			{
				Value = InProperty;
				SetDirty(true);
			}

			return *this;
		}
	};

}