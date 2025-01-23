#pragma once

#include <map>
#include <hermod/utilities/Types.h>
#include <hermod/Platform/Platform.h>

namespace proto
{
	class INetObject;
}

namespace serialization
{
	class HERMOD_API NetIdMapping
	{
	public:
		static NetIdMapping& Get();

		using NetObjectType = proto::INetObject*;
		using ObjectIdToInstanceContainer = std::map<NetObjectId, NetObjectType>;
		using ObjectInstanceToIdContainer = std::map<NetObjectType, NetObjectId>;

		void Clear();
		NetObjectId GetOrAssignedNetId(ObjectInstanceToIdContainer::key_type Value);
		NetObjectType GetObjectFromNetId(ObjectIdToInstanceContainer::key_type Value) const;
	private:
#pragma warning(push)
#pragma warning(disable : 4251)
		ObjectIdToInstanceContainer IdToObjectMap;
		ObjectInstanceToIdContainer ObjectToIdMap;
#pragma warning(pop)
	};
}