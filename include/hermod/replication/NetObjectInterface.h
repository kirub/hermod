#pragma once

#include <hermod/platform/Platform.h>
#include <hermod/serialization/Stream.h>
#include <hermod/replication/PropertyTypes.h>
#include <hermod/replication/NetObjectManager.h>
#include <hermod/replication/NetProperty.h>
#include <hermod/utilities/Utils.h>
#include <hermod/utilities/Types.h>

#include <functional>
#include <limits>
#include <memory>
#include <vector>
#include <tuple>
#include <hermod/serialization/FakeWriteStream.h>


namespace proto
{
	class INetProperty;


class HERMOD_API INetObject
{
	using PropertiesContainer = utils::FixedIntrusiveArray<MaxPropertyPerObject, INetProperty>;

	INTERNAL_NETCLASS_ID(INetObject)

protected:
	enum ENetObjectType
	{
		Fragment = 0,
		Object,
		Function,

		Count
	};
public:
	static const uint32_t CustomDataId = 0;

	INetObject();
	INetObject(ENetObjectType InNetObjectType );

	bool Serialize(serialization::IStream& Stream, std::optional<NetObjectManager::PropertiesListenerContainer> Mapper = std::optional<NetObjectManager::PropertiesListenerContainer>());
	bool SerializeProperties(serialization::IStream& Stream, std::optional<NetObjectManager::PropertiesListenerContainer> Mapper = std::optional<NetObjectManager::PropertiesListenerContainer>());

	void AddProperty(INetProperty& Property);
	void RemoveProperty(INetProperty& Property);

	virtual void OnReceived() {}
private:
	virtual bool SerializeImpl(serialization::IStream& Stream);

#pragma warning(push)
#pragma warning(disable : 4251)
	NetProperty<NetObjectId>	NetId;
	PropertiesContainer			Properties;
#pragma warning(pop)
}; 

class INetRPC
	: public INetObject
{
public:
	static uint32_t Id;

	INetRPC();

private:

	virtual void OnReceived() override
	{

	}

	NetProperty<uint32_t> Hash;
};

}