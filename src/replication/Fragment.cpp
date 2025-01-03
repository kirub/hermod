#include <hermod/protocol/Fragment.h>
#include <hermod/serialization/Stream.h>

namespace proto
{
	Fragment::Fragment()
		: Fragment(0u, 0u, nullptr, MaxFragmentSize)
	{
	}

	Fragment::Fragment(uint8_t InFragmentId, uint8_t InFragmentCount, unsigned char* InFragmentData, const std::size_t InFragmentSize)
		: Id(InFragmentId)
		, Count(InFragmentCount)
		, Data(new unsigned char[InFragmentSize])
		, DataSize(InFragmentSize)
	{
		if (InFragmentData)
		{
			memcpy(Data, InFragmentData, InFragmentSize);
		}
		NetObjectManager::Get().Register<Fragment>();
	}

	Fragment::~Fragment()
	{
		delete[] Data;
	}

	bool Fragment::SerializeImpl(serialization::IStream& Stream)
	{
		return Stream.Serialize(Data, { DataSize });
	}
}
