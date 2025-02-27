#pragma once

#include "Fragment.h"

namespace serialization
{
	class ReadStream;
	class WriteStream;
}
namespace proto
{
	class HERMOD_API FragmentHandler
	{
		uint8_t NumFragments;

		using FragmentPtr = std::shared_ptr<Fragment>;
		using FragmentContainer = std::deque<FragmentPtr>;
		const int Index(uint16_t Sequence) const;

	public:
		using ValueType = FragmentPtr;
#pragma warning(push)
#pragma warning(disable : 4251)
		FragmentContainer Entries;
#pragma warning(pop)


		const uint16_t InvalidEntry = 0xFFFF;
		const int MaxEntries = 256;

		void Reset();

		FragmentHandler();
		FragmentHandler(serialization::WriteStream& Stream, const std::size_t& MaxFragmentSize);
		FragmentHandler(unsigned char* InBuffer, int BufferSize, const std::size_t& MaxFragmentSize);
		FragmentHandler(proto::INetObject& InNetObject, const int BufferSize, const std::size_t& MaxFragmentSize);

		void Gather(serialization::ReadStream& OutStream) const;

		bool IsComplete() const;

		void OnFragment(FragmentPtr InFragment);
	};
}