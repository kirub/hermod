#pragma once

#include <hermod/replication/NetObjectInterface.h>
#include <execution>

namespace serialization
{
	class IStream;
}

namespace proto
{
	class HERMOD_API Fragment
		: public INetObject
	{
		CLASS_ID(Fragment)		
private:
	public:

		const uint8_t Invalid = 0xFF;
		uint8_t Id;
		uint8_t Count;
		const std::size_t DataSize;
		unsigned char* Data;

		Fragment();
		Fragment(uint8_t InFragmentId, uint8_t InFragmentCount, unsigned char* InFragmentData, const std::size_t InFragmentSize);

		virtual ~Fragment();

		virtual bool SerializeImpl(serialization::IStream& Stream) override;
	};
}