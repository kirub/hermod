#pragma once

#include <map>
#include <cassert>
#include <hermod/utilities/Types.h>
#include <hermod/Platform/Platform.h>

namespace proto
{
	class INetObject;
}

namespace serialization
{
	struct NetObjectData
	{
		unsigned char*	Buffer;
		std::size_t		BufferSize;

		NetObjectData();

		~NetObjectData();

		void Init(proto::INetObject& InObject);

		operator bool() const;
	};	
}