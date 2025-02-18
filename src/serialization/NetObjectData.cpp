#include <hermod/serialization/NetObjectData.h>
#include <hermod/serialization/FakeWriteStream.h>
#include <hermod/serialization/WriteStream.h>
#include <hermod/replication/NetObjectInterface.h>

namespace serialization
{
	NetObjectData::NetObjectData()
		: Buffer(nullptr)
		, BufferSize(0)
	{

	}

	NetObjectData::~NetObjectData()
	{
		delete[] Buffer;
	}

	void NetObjectData::Init(proto::INetObject& InObject)
	{
		FakeWriteStream MeasureStream;
		assert(InObject.Serialize(MeasureStream));
		//assert(MeasureStream.Align(32));
		MeasureStream.Flush();

		BufferSize = MeasureStream.GetDataSize();
		Buffer = new unsigned char[BufferSize];
		WriteStream Stream(Buffer, (int)BufferSize);

		assert(InObject.Serialize(Stream));
		//assert(Stream.Align(32));
		Stream.Flush();
	}

	NetObjectData::operator bool() const
	{
		return Buffer != nullptr && BufferSize > 0;
	}
}