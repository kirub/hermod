#pragma once
#include <hermod/replication/PropertyTypes.h>
#include <hermod/utilities/Utils.h>

namespace serialization
{
	class IStream;
}

namespace proto
{
	class HERMOD_API INetProperty
		: public utils::IIntrusiveElement< InvalidPropertyIndex >
	{
		class INetObject& Parent;
		bool	Dirty;

		virtual bool SerializeImpl(serialization::IStream& Stream) = 0;
	public:

		//INetProperty();
		INetProperty(class INetObject& InParent);

		bool IsDirty() const;
		void SetDirty(bool InDirtiness, bool NotifyParent = false);


		bool Serialize(serialization::IStream& Stream);
	};
}