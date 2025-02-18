#pragma once

#include <map>
#include <cassert>
#include <hermod/utilities/Types.h>
#include <hermod/Platform/Platform.h>
#include <hermod/serialization/NetObjectData.h>

namespace proto
{
	class INetObject;
}

namespace serialization
{
	class NetObjectSerializationCache
	{
	public:

		static NetObjectSerializationCache& Get();

		using ObjectIdToInstanceContainer = std::map<NetObjectId, NetObjectDataPtr>;

		void Clear();

		void ClearNetObjectData(NetObjectId ObjectId);
		NetObjectDataPtr GetNetObjectData(proto::INetObject& Object);
	private:
		NetObjectSerializationCache();
#pragma warning(push)
#pragma warning(disable : 4251)
		ObjectIdToInstanceContainer IdToDatatMap;
#pragma warning(pop)
	};
}