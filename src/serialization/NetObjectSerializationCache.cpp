#include <hermod/serialization/NetObjectSerializationCache.h>
#include <hermod/serialization/NetObjectData.h>
#include <hermod/replication/NetObjectInterface.h>

namespace serialization
{
	NetObjectSerializationCache::NetObjectSerializationCache()
		: IdToDatatMap()
	{
	}

	NetObjectSerializationCache& NetObjectSerializationCache::Get()
	{
		static NetObjectSerializationCache instance;
		return instance;
	}

	void NetObjectSerializationCache::Clear()
	{
		IdToDatatMap.clear();
	}
	
	void NetObjectSerializationCache::ClearNetObjectData(NetObjectId ObjectId)
	{
		IdToDatatMap.erase(ObjectId);
	}

	NetObjectDataPtr NetObjectSerializationCache::GetNetObjectData(proto::INetObject& Object)
	{
		NetObjectDataPtr& Data = IdToDatatMap[Object.GetId()];
		if (!Data || Object.IsDirty())
		{
			if (!Data)
			{
				Data = std::make_shared<NetObjectData>();
			}
			Data->Init(Object);
		}

		return Data;
	}
}