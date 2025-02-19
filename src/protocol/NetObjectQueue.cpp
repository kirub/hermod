#include <hermod/protocol/NetObjectQueue.h>
#include <hermod/protocol/Fragment.h>
#include <hermod/serialization/NetObjectSerializationCache.h>
#include <hermod/serialization/WriteStream.h>
#include <hermod/serialization/ReadStream.h>
#include <cassert>

namespace proto
{	
		
	NetObjectQueue::NetObjectQueue()
		: CurrentQueueIdx(0)
		, OldUnackedId(0)
	{
	}

	int NetObjectQueue::GetIdxFromId(int Id) const
	{
		return Id % QueueSizeMax;
	}
		
	int NetObjectQueue::IncIdx(int InInc /*= 1*/)
	{
		int Idx = CurrentQueueIdx;
		CurrentQueueIdx = GetNextIdx(InInc);
		assert(CurrentQueueIdx != OldUnackedId);
		return Idx;
	}
	
	int NetObjectQueue::GetNextIdx(int InInc /*= 1*/)
	{
		return GetIdxFromId(CurrentQueueIdx + InInc);
	}

	void NetObjectQueue::Clear()
	{
		for (NetQueueObjectData* ItData = Queue; ItData != (Queue+QueueSizeMax); ++ItData)
		{
			ItData->Reset();
		}
	}
	int32_t NetObjectQueue::Size() const
	{
		return CurrentQueueIdx - OldUnackedId;
	}

	bool NetObjectQueue::Empty() const
	{
		return Size() == 0;
	}
	
	void NetObjectQueue::OnMessagesAcked(std::vector<int> Ids)
	{
		for (int Id : Ids)
		{
			Queue[GetIdxFromId(Id)].Reset();
		}
		UpdateOldestUnackedId();
	}

	proto::NetQueueObjectData& NetObjectQueue::QueueObject(NetObjectPtr InNetObject, int MessageId /*= -1*/)
	{
		if (MessageId == -1)
		{
			MessageId = IncIdx();
		}
		proto::NetQueueObjectData& Data = Queue[GetIdxFromId(MessageId)];
		Data.Grab(MessageId, InNetObject);
		return Data;

	}
	NetObjectPtr NetObjectQueue::DequeueObject()
	{
		if (NetQueueObjectData& Data = Queue[CurrentQueueIdx])
		{
			NetObjectPtr ReturnObject = Data.Object;
			IncIdx(Data.SpaceCount);
			Data.Reset();
			return ReturnObject;
		}

		return nullptr;
	}
	
	void NetObjectQueue::AddForSend(NetObjectPtr InNetObject)
	{
		QueueObject(InNetObject);

	}
	void NetObjectQueue::AddForReceive(int MessageId, NetObjectPtr InNetObject, int MessageSpaceCount /*= 1*/)
	{
		proto::NetQueueObjectData& Data = QueueObject(InNetObject, MessageId);
		Data.SpaceCount = MessageSpaceCount;
	}
	
	int NetObjectQueue::UpdateOldestUnackedId()
	{
		int Idx = OldUnackedId;
		NetQueueObjectData CurrentData = Queue[Idx];
		while (!CurrentData && Idx != CurrentQueueIdx)
		{
			if (CurrentData)
			{
				OldUnackedId = Idx;
			}
			else
			{
				Idx = GetIdxFromId(Idx+1);
				CurrentData = Queue[Idx];
			}
		}
		return Idx;
	}

	uint8_t NetObjectQueue::ReadMessageId(serialization::ReadStream& InStream)
	{
		uint8_t NetObjectOrderId = 0;
		assert(InStream.Serialize(NetObjectOrderId));
		assert(InStream.Align(32));
		return NetObjectOrderId;
	}
	void NetObjectQueue::WriteMessageId(serialization::WriteStream& InStream, uint8_t MessageId)
	{
		assert(InStream.Serialize(MessageId));
		assert(InStream.Align(32));
	}

	int NetObjectQueue::GetSendBuffer(serialization::WriteStream& OutStream, std::vector<int>& Ids)
	{
		int NbMessagesHandle = 0;
		int Idx = OldUnackedId;
		while(Idx < CurrentQueueIdx)
		{
			NetQueueObjectData& Message = Queue[Idx];
			if (!Message.HasBeenSentLast(0.1f))
			{
				serialization::NetObjectDataPtr ObjectData = Message.GetObjectData();
				if(ObjectData && OutStream.GetBytesRemaining() > (ObjectData->BufferSize + 1))
				{
					Message.Touch();

					constexpr bool DoNotifyProperties = true;
					Message.Object->SetDirty(false, DoNotifyProperties);

					Ids.push_back(Message.Id);

					NetObjectQueue::WriteMessageId(OutStream, Message.Id);
					OutStream.InsertObjectBuffer(ObjectData);

					NbMessagesHandle += 1;
				}
			}
			++Idx;
		}

		return NbMessagesHandle;
	}
}