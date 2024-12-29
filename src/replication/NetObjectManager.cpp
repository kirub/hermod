#include <hermod/replication/NetObjectManager.h>

#include <hermod/replication/NetObjectInterface.h>
#include <hermod/serialization/ReadStream.h>

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

NetObjectManager::RetNetObjectType NetObjectManager::HandlePacket(serialization::ReadStream& Reader)
{
    // process packet
    uint32_t NetObjectId = 0;
    if (Reader.Serialize(NetObjectId) && NetObjectId != 0)
    {
        // Try instantiate packet
        if (RetNetObjectType NewNetObject = NetObjectManager::Get().Instantiate(NetObjectId))
        {
            std::optional<PropertiesListenerContainer> PropertiesListener;
            ObjectsListenerContainer::const_iterator itFoundObjectListener = ObjectListeners.find(NetObjectId);
            if (itFoundObjectListener != ObjectListeners.end())
            {
                PropertiesListener = itFoundObjectListener->second;
            }

            assert(NewNetObject->Serialize(Reader, PropertiesListener));
            return NewNetObject;
        }
    }

    return nullptr;
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