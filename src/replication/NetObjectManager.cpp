#include <hermod/replication/NetObjectManager.h>

#include <hermod/protocol/ConnectionInterface.h>
#include <hermod/replication/NetPropertyInterface.h>
#include <hermod/replication/NetObjectInterface.h>
#include <hermod/serialization/ReadStream.h>
#include <hermod/serialization/WriteStream.h>

#include <utility>
#include <memory>

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

NetObjectManager::NetObjectType NetObjectManager::Instantiate(const uint32_t ObjectClassId) const
{
    ObjectsConstructorContainer::const_iterator itFound = Factory.find(ObjectClassId);
    if (itFound == Factory.end())
    {
        return nullptr;
    }

    return std::shared_ptr<proto::INetObject>(itFound->second());
}

void NetObjectManager::ReplicateObjects(std::vector < ConnectionPtr > Connections)
{
    for_each(Connections.begin(), Connections.end(), std::bind(&IConnection::BuildConsiderList, std::placeholders::_1, NetObjects));
}

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