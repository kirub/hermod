#include <hermod/replication/NetObjectManager.h>

#include <hermod/protocol/ConnectionInterface.h>
#include <hermod/replication/NetObjectInterface.h>
#include <hermod/serialization/ReadStream.h>
#include <hermod/serialization/WriteStream.h>

#include <utility>

NetObjectManager& NetObjectManager::Get() {
    static NetObjectManager instance;
    return instance;
}

void NetObjectManager::Unregister(const uint32_t ObjectClassId)
{
    Factory.erase(ObjectClassId);
}

uint32_t NetObjectManager::NetObjectsCount() const
{
    return static_cast<uint32_t>(Factory.size());
}

NetObjectManager::RetNetObjectType NetObjectManager::Instantiate(const uint32_t ObjectClassId) const
{
    ObjectsConstructorContainer::const_iterator itFound = Factory.find(ObjectClassId);
    if (itFound == Factory.end())
    {
        return nullptr;
    }

    return itFound->second();
}

void NetObjectManager::ReplicateObjects(std::vector < std::shared_ptr < IConnection >> Connections)
{
    for (std::shared_ptr<IConnection> Connection : Connections)
    {
        std::vector< std::shared_ptr<proto::INetObject>> NetObjectListFiltered;// = Connection->BuildConsiderList(NetObjects);

        for (std::shared_ptr<proto::INetObject> NetObject : NetObjectListFiltered)
        {
            Connection->Send(*NetObject);
        }
    }
}
/*
bool NetObjectManager::SerializeObject(proto::INetObject*& NetObject, serialization::IStream& Stream)
{
    bool HasError = false;
    uint32_t NetObjectClassId = 0;
    if (Stream.IsWriting())
    {
        NetObjectClassId = NetObject->GetClassId();
    }
    HasError = !Stream.Serialize(NetObjectClassId);
    if (Stream.IsReading())
    {
        NetObject = NetObjectManager::Get().Instantiate(NetObjectClassId);
    }

    std::optional<PropertiesListenerContainer> PropertiesListener;
    ObjectsListenerContainer::const_iterator itFoundObjectListener = ObjectListeners.find(NetObjectClassId);
    if (itFoundObjectListener != ObjectListeners.end())
    {
        PropertiesListener = itFoundObjectListener->second;
    }

    HasError &= !NetObject->SerializeProperties(Stream, PropertiesListener);
    HasError &= !NetObject->SerializeImpl(Stream);

    if (!HasError && Stream.IsReading())
    {
        NetObject->OnReceived();
    }

    return HasError;
}*/
std::optional<NetObjectManager::PropertiesListenerContainer> NetObjectManager::GetPropertiesListeners(proto::INetObject& NetObject) const
{
    std::optional<PropertiesListenerContainer> PropertiesListener;
    ObjectsListenerContainer::const_iterator itFoundObjectListener = ObjectListeners.find(NetObject.GetClassId());
    if (itFoundObjectListener != ObjectListeners.end())
    {
        PropertiesListener = itFoundObjectListener->second;
    }

    return PropertiesListener;
}