#pragma once

#include <cassert>
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

		NetQueueObjectData& operator=(NetObjectPtr InObject);

		void Grab(int InId, NetObjectPtr InObject);
		serialization::NetObjectDataPtr GetObjectData() const;

		operator bool() const;

		void Reset(uint8_t InId = -1);

		void Touch();

		bool NeedsSendingAccordingToFrequency();
		bool HasBeenSentLast(int64_t TimeOffsetMs);
	};	
}