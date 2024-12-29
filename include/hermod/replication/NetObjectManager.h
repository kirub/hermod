#pragma once

#include <hermod/replication/NetPropertyInterface.h>
#include <hermod/serialization/ReadStream.h>

#include <functional>
#include <map>
#include <memory>
#include <hermod/protocol/ConnectionInterface.h>

namespace proto
{
    class ReadStream;
    class INetObject;
}

class NetObjectManager
{
    NetObjectManager() = default;
public:
    using RetNetObjectType = proto::INetObject*;
    using ObjectConstructor = std::function<RetNetObjectType()>;
    template < std::derived_from<proto::INetProperty> T>
    using PropertyListenerWithT = std::function<void(const T&)>;
    using PropertyListener = std::function<void(const proto::INetProperty&)>;
    using ObjectsConstructorContainer = std::map< uint32_t, ObjectConstructor>;
    using PropertiesListenerContainer = std::map< uint8_t, PropertyListener>;
    using ObjectsListenerContainer = std::map< uint32_t, PropertiesListenerContainer>;

    HERMOD_API static NetObjectManager& Get();

    template < std::derived_from<proto::INetObject> T>
    void Register(const ObjectConstructor& Contructor = []() { return new T(); })
    {
        Factory.insert({ T::StaticClassId(), Contructor });
    }


    template < std::derived_from<proto::INetObject> NetObject, std::derived_from<proto::INetProperty> PropType>
    bool RegisterPropertyListener(const proto::INetProperty& Property, const PropertyListenerWithT<PropType>& Listener)
    {
        return ObjectListeners.insert({ NetObject::StaticClassId(), { Property.GetIndex(), Listener} }).second;
    }

    void Unregister(const uint32_t ObjectClassId);

    uint32_t NetObjectsCount() const;

    RetNetObjectType Instantiate(const uint32_t ObjectClassId) const;

    HERMOD_API RetNetObjectType HandlePacket(serialization::ReadStream& Reader);

    void ReplicateObjects(std::vector < std::shared_ptr < IConnection >> Connections);

private:

    ObjectsConstructorContainer Factory;
    ObjectsListenerContainer ObjectListeners;
    std::vector<std::shared_ptr<proto::INetObject>> NetObjects;
};

