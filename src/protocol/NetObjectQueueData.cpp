#include <hermod/protocol/NetObjectQueueData.h>
#include <hermod/serialization/NetObjectSerializationCache.h>
#include <hermod/serialization/NetObjectData.h>

namespace proto
{
	NetQueueObjectData::NetQueueObjectData()
		: Id(-1)
		, Object(nullptr)
		, TimeSent(0)
		, SpaceCount(1)
	{ 
	}

	void NetQueueObjectData::Grab(int InId, NetObjectPtr InObject)
	{
		Id = InId;
		Object = InObject;
	}

	NetQueueObjectData::operator bool() const
	{
		return Id > -1 && Object;
	}

	void NetQueueObjectData::Reset()
	{
		Id = -1;
		//serialization::NetObjectSerializationCache::Get().ClearNetObjectData(Id);
		Object.reset();
		TimeSent = 0;
		SpaceCount = 1;
	}

	serialization::NetObjectDataPtr NetQueueObjectData::GetObjectData() const
	{
		return serialization::NetObjectSerializationCache::Get().GetNetObjectData(*Object);
	}

	void NetQueueObjectData::Touch()
	{
		TimeSent = utils::Time::NowMs();
	}

	bool NetQueueObjectData::HasBeenSentLast(float TimeOffsetSec)
	{
		return utils::Time::NowMs() - TimeSent < (TimeOffsetSec*1000.0f);
	}
}