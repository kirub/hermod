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
		NumFragments = Stream.GetDataSize() / MaxFragmentSize;
		NumFragments += ((Stream.GetDataSize() % MaxFragmentSize) == 0) ? 0 : 1;
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
		unsigned char* DestBuffer = new unsigned char[MaxFragmentSize * NumFragments];
		std::size_t Offset = 0;
		unsigned char* NotUsed = std::accumulate(Entries.begin(), Entries.end(), DestBuffer,
			[&Offset, MaxSize = std::size_t(MaxFragmentSize * NumFragments)](unsigned char* StreamBuffer, FragmentPtr InFragment)
			{
				assert(Offset + InFragment->DataSize < MaxSize);
				Offset += InFragment->DataSize;
				memcpy(StreamBuffer, InFragment->Data, InFragment->DataSize);
				StreamBuffer += InFragment->DataSize;
				return StreamBuffer;
			}
		);
		return { DestBuffer, MaxFragmentSize * NumFragments, [](unsigned char* ptr) { delete[] ptr; } };
	}

	bool FragmentHandler::IsComplete() const
	{
		auto* start = Entries.data();
		auto* end = start + NumFragments;
		return std::find(start, end, nullptr) == end;
	}

	void FragmentHandler::OnFragment(FragmentPtr InFragment)
	{
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
	}
}