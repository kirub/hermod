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

		using FragmentPtr = Fragment*;
		using FragmentContainer = std::vector<FragmentPtr>;
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

		serialization::ReadStream Gather();

		bool IsComplete() const;

		void OnFragment(uint8_t FragmentCount, uint8_t FragmentId, FragmentPtr InFragment);
	};
}