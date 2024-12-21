#pragma once

#include <hermod/replication/NetPropertyInterface.h>
#include <hermod/serialization/ReadStream.h>

#include <functional>
#include <map>
#include <memory>

namespace proto
{
    class ReadStream;
    class INetObject;
}

class NetObjectManager
{
    NetObjectManager() = default;
public:
    using ObjectConstructor = std::function<std::unique_ptr<proto::INetObject>()>;
    template < std::derived_from<proto::INetProperty> T>
    using PropertyListenerWithT = std::function<void(const T&)>;
    using PropertyListener = std::function<void(const proto::INetProperty&)>;
    using ObjectsConstructorContainer = std::map< uint32_t, ObjectConstructor>;
    using PropertiesListenerContainer = std::map< uint8_t, PropertyListener>;
    using ObjectsListenerContainer = std::map< uint32_t, PropertiesListenerContainer>;

    static NetObjectManager& Get();

    template < std::derived_from<proto::INetObject> T>
    bool Register()
    {
        return Factory.insert({ T::StaticClassId(), []() { return std::make_unique<T>(); } }).second;
    }


    template < std::derived_from<proto::INetObject> NetObject, std::derived_from<proto::INetProperty> PropType>
    bool RegisterPropertyListener(const proto::INetProperty& Property, const PropertyListenerWithT<PropType>& Listener)
    {
        return ObjectListeners.insert({ NetObject::StaticClassId(), { Property.GetIndex(), Listener} }).second;
    }

    void Unregister(const uint32_t ObjectClassId);

    uint32_t NetObjectsCount() const;

    std::unique_ptr<proto::INetObject> Instantiate(const uint32_t ObjectClassId) const;

    bool HandlePacket(serialization::ReadStream& Reader);

private:

    ObjectsConstructorContainer Factory;
    ObjectsListenerContainer ObjectListeners;
};

