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
		for (FragmentPtr Frag : Entries)
		{
			delete Frag;
		}
		Entries.clear();
	}

	FragmentHandler::FragmentHandler()
	{
		Reset();
	}

	FragmentHandler::FragmentHandler(serialization::WriteStream& Stream, const std::size_t& MaxFragmentSize)
		: NumFragments((Stream.GetDataSize() / MaxFragmentSize) + ((Stream.GetDataSize() % MaxFragmentSize) == 0) ? 0 : 1)
		, Entries(NumFragments)
	{
		NumFragments = (uint8_t) (Stream.GetDataSize() / MaxFragmentSize);
		NumFragments += ((Stream.GetDataSize() % MaxFragmentSize) == 0) ? 0 : 1;
		Entries.reserve(NumFragments);
		unsigned char* DataStart = (unsigned char*)Stream.GetData();
		const int DataSize = Stream.GetDataSize();
		std::size_t ProcessedBytes = 0;
		for (int FragmentId = 0; FragmentId < NumFragments; ++FragmentId)
		{
			std::size_t NumBytesToCopy = std::min(MaxFragmentSize, (std::size_t)(DataSize - ProcessedBytes));
			Entries.emplace_back(new Fragment((uint8_t)FragmentId, NumFragments, DataStart + ProcessedBytes, NumBytesToCopy));
			ProcessedBytes += NumBytesToCopy;
		}
	}

	serialization::ReadStream FragmentHandler::Gather()
	{
		int MaxSize = MaxFragmentSize * NumFragments;
		serialization::ReadStream Stream(MaxSize);
		unsigned char*  DestBuffer = (unsigned char*)Stream.GetData();
		int Offset = 0;
		for (FragmentPtr InFragment : Entries)
		{
			assert(Offset + (int)InFragment->DataSize <= MaxSize);
			memcpy(DestBuffer + Offset, InFragment->Data, InFragment->DataSize);
			Offset += (int)InFragment->DataSize;
		}	
		return Stream;
	}

	bool FragmentHandler::IsComplete() const
	{
		auto* start = Entries.data();
		auto* end = start + NumFragments;
		return std::find(start, end, nullptr) == end;
	}

	void FragmentHandler::OnFragment(FragmentPtr InFragment)
	{
		assert(InFragment);
		if (Entries.capacity() == 0)
		{
			Entries.resize(InFragment->Count);
			NumFragments = InFragment->Count;
		}
		else
		{
			assert(NumFragments == InFragment->Count);
			assert(Entries.capacity() == NumFragments);
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