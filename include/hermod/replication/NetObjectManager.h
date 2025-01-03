#pragma once

#include <hermod/replication/NetPropertyInterface.h>

#include <functional>
#include <map>
#include <memory>

class IConnection;

namespace serialization
{
    class IStream;
    class ReadStream;
    class WriteStream;
}

namespace proto
{
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
        Factory.insert({ T::NetObjectId::value, Contructor });
    }


    template < std::derived_from<proto::INetObject> NetObject, std::derived_from<proto::INetProperty> PropType>
    bool RegisterPropertyListener(const proto::INetProperty& Property, const PropertyListenerWithT<PropType>& Listener)
    {
        return ObjectListeners.insert({ NetObject::NetObjectId::value, { Property.GetIndex(), Listener} }).second;
    }

    void Unregister(const uint32_t ObjectClassId);

    uint32_t NetObjectsCount() const;

    HERMOD_API RetNetObjectType Instantiate(const uint32_t ObjectClassId) const;
    template < typename... Args >
    RetNetObjectType Instantiate(const uint32_t ObjectClassId, Args&&... InArgs) const
    {
        ObjectsConstructorContainer::const_iterator itFound = Factory.find(ObjectClassId);
        if (itFound == Factory.end())
        {
            return nullptr;
        }

        return itFound->second(std::forward<Args>(InArgs)...);
    }

    //HERMOD_API RetNetObjectType HandlePacket(serialization::ReadStream& Reader);

    void ReplicateObjects(std::vector < std::shared_ptr < IConnection >> Connections);
    //HERMOD_API bool SerializeObject(proto::INetObject*& NetObject, serialization::IStream& Stream);

    HERMOD_API std::optional<PropertiesListenerContainer> GetPropertiesListeners(proto::INetObject& NetObject) const;


private:

    ObjectsConstructorContainer Factory;
    ObjectsListenerContainer ObjectListeners;
    std::vector<std::shared_ptr<proto::INetObject>> NetObjects;
};

