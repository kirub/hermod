#include <hermod/protocol/Fragment.h>
#include <hermod/serialization/Stream.h>

namespace proto
{
	Fragment::Fragment()
		: Fragment(0u, 0u, nullptr, MaxFragmentSize)
	{
	}

	Fragment::Fragment(uint8_t InFragmentId, uint8_t InFragmentCount, unsigned char* InFragmentData, const std::size_t InFragmentSize)
		: Fragment(InFragmentId, InFragmentCount, InFragmentData, InFragmentSize, false)
	{
	}

	Fragment::Fragment(uint8_t InFragmentId, uint8_t InFragmentCount, unsigned char* InFragmentData, const std::size_t InFragmentSize, bool StealData)
		: INetObject(INetObject::Fragment)
		, Id(InFragmentId)
		, Count(InFragmentCount)
		, Data(StealData ? InFragmentData : new unsigned char[InFragmentSize])
		, DataSize(InFragmentSize)
	{
		if (!StealData && InFragmentData)
		{
			memcpy(Data, InFragmentData, InFragmentSize);
		}
		NetObjectManager::Get().Register<Fragment>();
	}

	Fragment::~Fragment()
	{
		if (Data && DataSize)
		{
			delete[] Data;
		}
	}

	bool Fragment::SerializeImpl(serialization::IStream& Stream)
	{
		return Stream.Serialize(Count) &&
			Stream.Serialize(Id, { Count }) &&
			Stream.Serialize(Data, { DataSize });
	}
}
