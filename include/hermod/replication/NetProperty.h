#pragma once
#include <hermod/serialization/Stream.h>
#include <hermod/replication/NetPropertyInterface.h>

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

		NetProperty(INetObject& InParent, const PropertyType InValue)
			: NetProperty(InParent, InValue, serialization::NetPropertySettings<PropertyType>())
		{
		}
		NetProperty(INetObject& InParent, const PropertyType InValue, const serialization::NetPropertySettings<PropertyType>& InProperties)
			: INetProperty(InParent)
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
				constexpr bool DontNotifyParent = false;
				SetDirty(true, DontNotifyParent);
			}

			return *this;
		}
	};

}