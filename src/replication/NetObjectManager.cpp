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

std::unique_ptr<proto::INetObject> NetObjectManager::Instantiate(const uint32_t ObjectClassId) const
{
    ObjectsConstructorContainer::const_iterator itFound = Factory.find(ObjectClassId);
    if (itFound == Factory.end())
    {
        return nullptr;
    }

    return itFound->second();
}

bool NetObjectManager::HandlePacket(serialization::ReadStream& Reader)
{
    // process packet
    uint32_t NetObjectId = 0;
    if (!Reader.Serialize(NetObjectId) || NetObjectId == 0)
    {
        return false;
    }

    // Try instantiate packet
    if (std::unique_ptr<proto::INetObject> NewNetObject = NetObjectManager::Get().Instantiate(NetObjectId))
    {
        std::optional<PropertiesListenerContainer> PropertiesListener;
        ObjectsListenerContainer::const_iterator itFoundObjectListener = ObjectListeners.find(NetObjectId);
        if (itFoundObjectListener != ObjectListeners.end())
        {
            PropertiesListener = itFoundObjectListener->second;
        }

        return NewNetObject->Serialize(Reader, PropertiesListener);
    }

    return false;
}