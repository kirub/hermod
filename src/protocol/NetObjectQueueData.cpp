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

	void NetQueueObjectData::Reset(uint8_t InId /*= -1*/)
	{
		Id = InId;
		//serialization::NetObjectSerializationCache::Get().ClearNetObjectData(Id);
		Object.reset();
		TimeSent = 0;
		SpaceCount = 1;
	}

	NetQueueObjectData& NetQueueObjectData::operator=(NetObjectPtr InObject)
	{
		assert(Id != -1);
		Object = InObject;
		TimeSent = 0;
		SpaceCount = 1;

		return *this;
	}

	serialization::NetObjectDataPtr NetQueueObjectData::GetObjectData() const
	{
		return serialization::NetObjectSerializationCache::Get().GetNetObjectData(*Object);
	}

	void NetQueueObjectData::Touch()
	{
		TimeSent = utils::Time::NowMs();
	}

	bool NetQueueObjectData::NeedsSendingAccordingToFrequency()
	{
		assert(Object);
		return HasBeenSentLast(Object->GetUpdateFrequency());
	}

	bool NetQueueObjectData::HasBeenSentLast(int64_t TimeOffsetMs)
	{
		return utils::Time::NowMs() - (int64_t)TimeSent < TimeOffsetMs;
	}
}