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
		std::function<void(const PropertyType& InValue)> OnSerializeCallback;


		virtual bool SerializeImpl(serialization::IStream& Stream)
		{
			if (OnSerializeCallback)
			{
				OnSerializeCallback(Value);
			}
			return Stream.Serialize(Value, Properties);
		}

	public:

		NetProperty(const PropertyType InValue)
			: NetProperty(InValue, serialization::NetPropertySettings<PropertyType>())
		{
		}
		NetProperty(const PropertyType InValue, const serialization::NetPropertySettings<PropertyType>& InProperties)
			: INetProperty()
			, Properties(InProperties)
			, Value(InValue)
			, OnSerializeCallback()
		{
			operator=(InValue);
		}

		void OnSerialize(std::function<void(const PropertyType& InValue)> InCallback)
		{
			OnSerializeCallback = InCallback;
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