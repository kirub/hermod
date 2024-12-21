#pragma once
#include <hermod/replication/PropertyTypes.h>
#include <hermod/serialization/Stream.h>
#include <hermod/utilities/Utils.h>

namespace proto
{
	class HERMOD_API INetProperty
		: public utils::IIntrusiveElement< InvalidPropertyIndex >
	{
		bool	Dirty;

		virtual bool SerializeImpl(serialization::IStream& Stream) = 0;
	public:

		INetProperty();
		INetProperty(class INetObject& InPacket);

		bool IsDirty() const;
		void SetDirty(bool InDirtiness);


		bool Serialize(serialization::IStream& Stream);
	};
}