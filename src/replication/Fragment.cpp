#include <hermod/protocol/Fragment.h>
#include <hermod/serialization/Stream.h>

namespace proto
{
	Fragment::Fragment()
		: Fragment(0u, 0u, nullptr, MaxPacketSize)
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

	bool Fragment::Serialize(serialization::IStream& Stream, std::optional<NetObjectManager::PropertiesListenerContainer> Mapper /*= std::optional<NetObjectManager::PropertiesListenerContainer>()*/)
	{
		return Stream.Serialize(Count) &&
			Stream.Serialize(Id, { Count }) &&
			Stream.Serialize(Data, { DataSize });
	}
}
