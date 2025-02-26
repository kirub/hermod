#pragma once

#include <hermod/replication/NetObjectInterface.h>
#include <hermod/protocol/NetObjectQueueData.h>

namespace serialization
{
    class ReadStream;
    class WriteStream;
}
namespace proto
{	
	class NetObjectQueue
	{
		static const int QueueSizeMax = 255;

		int OldUnackedId;
		int CurrentQueueIdx;
		NetQueueObjectData Queue[QueueSizeMax];

        int GetNextIdx(int InInc = 1);
		int GetIdxFromId(int Id) const;

		int IncIdx(int InInc = 1);

        proto::NetQueueObjectData& QueueObject(NetObjectPtr InNetObject, int MessageId = -1);
	public:

        class iterator;
        class const_iterator;

		NetObjectQueue();

        void Clear();
        int32_t Size() const;
        bool Empty() const;
        void Advance(iterator Target);

		void OnMessagesAcked(std::vector<int> Ids);

		void AddForSend(NetObjectPtr InNetObject);
		void AddForReceive(int MessageId, NetObjectPtr InNetObject, int MessageSpaceCount = 1);


        static uint8_t ReadMessageId(serialization::ReadStream& InStream);
        static void WriteMessageId(serialization::WriteStream& InStream, uint8_t MessageId);

		int UpdateOldestUnackedId();

        NetObjectPtr DequeueObject();
		int GetSendBuffer(serialization::WriteStream& OutStream, std::vector<uint8_t>& Ids);

        class iterator {
        public:
            // iterator traits
            using difference_type = std::size_t;
            using value_type = NetQueueObjectData&;
            using pointer = NetQueueObjectData*;
            using reference = NetQueueObjectData&;
            using iterator_category = std::forward_iterator_tag;

            iterator(pointer StartFrom) : Current(StartFrom) {}
            iterator& operator++() { Current++; return *this; }
            iterator operator++(int) { iterator retval = *this; ++(*this); return retval; }
            bool operator==(iterator other) const { return Current == other.Current; }
            bool operator!=(iterator other) const { return !(*this == other); }
            value_type operator*() { return *Current; }
        private:
            pointer Current;
        };
        class const_iterator {
        public:
            // const_iterator traits
            using difference_type = std::size_t;
            using value_type = const NetQueueObjectData&;
            using pointer = const NetQueueObjectData*;
            using reference = const NetQueueObjectData&;
            using iterator_category = std::forward_iterator_tag;

            const_iterator(const pointer StartFrom) : Current(StartFrom) {}
            const_iterator& operator++() { Current++; return *this; }
            const_iterator operator++(int) { const_iterator retval = *this; ++(*this); return retval; }
            bool operator==(const_iterator other) const { return Current == other.Current; }
            bool operator!=(const_iterator other) const { return !(*this == other); }
            value_type operator*() { return *Current; }
        private:
            pointer Current;
        };

        iterator begin() { return Queue; }
        iterator first_free() { return Queue + CurrentQueueIdx; }
        iterator end() { return Queue + QueueSizeMax; }
        const_iterator cbegin() { return Queue; }
        const_iterator cend() { return Queue + QueueSizeMax; }
	};
}