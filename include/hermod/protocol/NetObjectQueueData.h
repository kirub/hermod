#pragma once

#include <hermod/replication/NetObjectInterface.h>

namespace proto
{
	struct NetQueueObjectData
	{
		uint64_t TimeSent;
		uint8_t Id;
		uint8_t SpaceCount;
		NetObjectPtr Object;

		NetQueueObjectData();

		void Grab(int InId, NetObjectPtr InObject);
		serialization::NetObjectDataPtr GetObjectData() const;

		operator bool() const;

		void Reset();

		void Touch();

		bool HasBeenSentLast(float TimeOffsetSec);
	};	
}