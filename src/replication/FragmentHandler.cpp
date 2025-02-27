#include <hermod/protocol/FragmentHandler.h>
#include <hermod/serialization/ReadStream.h>
#include <hermod/serialization/WriteStream.h>

namespace proto
{

	const int FragmentHandler::Index(uint16_t Sequence) const
	{
		return Sequence % NumFragments;
	}

	void FragmentHandler::Reset()
	{
		NumFragments = 0;		
		Entries.clear();
	}

	FragmentHandler::FragmentHandler()
	{
		Reset();
	}

	FragmentHandler::FragmentHandler(serialization::WriteStream& Stream, const std::size_t& MaxFragmentSize)
		: NumFragments((uint8_t)((Stream.GetDataSize() / (int)MaxFragmentSize) + ((Stream.GetDataSize() % (int)MaxFragmentSize) == 0 ? 0 : 1)))
		, Entries((std::size_t)NumFragments)
	{
		//NumFragments = (uint8_t) (Stream.GetDataSize() / MaxFragmentSize);
		//NumFragments += ((Stream.GetDataSize() % MaxFragmentSize) == 0) ? 0 : 1;
		//Entries.c.resize(NumFragments);
		unsigned char* DataStart = (unsigned char*)Stream.GetData();
		const int DataSize = Stream.GetDataSize();
		std::size_t ProcessedBytes = 0;
		for (int FragmentId = 0; FragmentId < NumFragments; ++FragmentId)
		{
			std::size_t NumBytesToCopy = std::min(MaxFragmentSize, (std::size_t)(DataSize - ProcessedBytes));
			Entries[FragmentId] = std::make_shared<Fragment>((uint8_t)FragmentId, NumFragments, DataStart + ProcessedBytes, NumBytesToCopy);
			ProcessedBytes += NumBytesToCopy;
		}
	}
	FragmentHandler::FragmentHandler(unsigned char* InBuffer, int BufferSize, const std::size_t& MaxFragmentSize)
		: NumFragments((uint8_t)((BufferSize / (int)MaxFragmentSize) + ((BufferSize % (int)MaxFragmentSize) == 0 ? 0 : 1)))
		, Entries((std::size_t)NumFragments)
	{
		//NumFragments = (uint8_t) (Stream.GetDataSize() / MaxFragmentSize);
		//NumFragments += ((Stream.GetDataSize() % MaxFragmentSize) == 0) ? 0 : 1;
		//Entries.c.resize(NumFragments);
		unsigned char* DataStart = InBuffer;
		const int DataSize = BufferSize;
		std::size_t ProcessedBytes = 0;
		constexpr bool StealData = true;
		for (int FragmentId = 0; FragmentId < NumFragments; ++FragmentId)
		{
			std::size_t NumBytesToCopy = std::min(MaxFragmentSize, (std::size_t)(DataSize - ProcessedBytes));
			Entries[FragmentId] = std::make_shared<Fragment>((uint8_t)FragmentId, NumFragments, DataStart + ProcessedBytes, NumBytesToCopy, StealData);
			ProcessedBytes += NumBytesToCopy;
		}
	}
	FragmentHandler::FragmentHandler(proto::INetObject& InNetObject, const int DataSize, const std::size_t& MaxFragmentSize)
		: NumFragments((uint8_t)((DataSize / (int)MaxFragmentSize) + ((DataSize % (int)MaxFragmentSize) == 0 ? 0 : 1)))
		, Entries((std::size_t)NumFragments)
	{
		//NumFragments = (uint8_t) (Stream.GetDataSize() / MaxFragmentSize);
		//NumFragments += ((Stream.GetDataSize() % MaxFragmentSize) == 0) ? 0 : 1;
		//Entries.c.resize(NumFragments);

		unsigned char* DataStart = new unsigned char[DataSize];
		serialization::WriteStream Stream(DataStart, DataSize);
		if (InNetObject.Serialize(Stream))
		{
			assert(Stream.Flush());
			assert(Stream.GetDataSize() == DataSize);
			std::size_t ProcessedBytes = 0;
			constexpr bool StealData = false;
			for (int FragmentId = 0; FragmentId < NumFragments; ++FragmentId)
			{
				std::size_t NumBytesToCopy = std::min(MaxFragmentSize, (std::size_t)(DataSize - ProcessedBytes));
				Entries[FragmentId] = std::make_shared<Fragment>((uint8_t)FragmentId, NumFragments, DataStart + ProcessedBytes, NumBytesToCopy, StealData);
				ProcessedBytes += NumBytesToCopy;
			}
		}
		delete[] DataStart;
	}

	void FragmentHandler::Gather(serialization::ReadStream& OutStream) const
	{
		//int MaxSize = MaxFragmentSize * NumFragments;
		//serialization::ReadStream Stream(MaxSize);
		unsigned char*  DestBuffer = (unsigned char*)OutStream.GetData();
		int Offset = 0;
		for (FragmentPtr InFragment : Entries)
		{
			assert(!OutStream.WouldOverflow((int)InFragment->DataSize));
			memcpy(DestBuffer + Offset, InFragment->Data, InFragment->DataSize);
			Offset += (int)InFragment->DataSize;
		}	
	}

	bool FragmentHandler::IsComplete() const
	{
		return std::find(Entries.begin(), Entries.end(), nullptr) == Entries.end();
	}

	void FragmentHandler::OnFragment(FragmentPtr InFragment)
	{
		assert(InFragment);
		if (Entries.empty())
		{
			Entries.resize(InFragment->Count);
			NumFragments = InFragment->Count;
		}
		else
		{
			assert(NumFragments == InFragment->Count);
			assert(Entries.size() == NumFragments);
		}
		Entries[Index(InFragment->Id)] = InFragment;
	}

	/*void FragmentHandler::OnFragment(uint8_t FragmentCount, uint8_t FragmentId, FragmentPtr InFragment)
	{
		InFragment->Count = FragmentCount;
		InFragment->Id = FragmentId;

		if (Entries.capacity() == 0)
		{
			Entries.resize(InFragment->Count);
			NumFragments = InFragment->Count;
		}
		else
		{
			assert(NumFragments == InFragment->Count);
			assert(Entries.max_size() == NumFragments);
		}
		Entries[Index(InFragment->Id)] = std::move(InFragment);
	}*/
}