#pragma once

#include <hermod/Platform/Platform.h>
#include <hermod/utilities/Types.h>
#include <functional>
#include <map>
#include <memory>
#include <optional>

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
    class INetProperty;
}

class NetObjectManager
{
    NetObjectManager() = default;
public:
    using NetObjectType = proto::INetObject*;
    using ObjectConstructor = std::function<NetObjectType()>;
    template < std::derived_from<proto::INetProperty> T>
    using PropertyListenerWithT = std::function<void(const T&)>;
    using PropertyListener = std::function<void(const proto::INetProperty&)>;
    using ObjectsConstructorContainer = std::map< uint32_t, ObjectConstructor>;
    using PropertiesListenerContainer = std::map< uint8_t, PropertyListener>;
    using ObjectsListenerContainer = std::map< uint32_t, PropertiesListenerContainer>;

    HERMOD_API static NetObjectManager& Get();

    template < std::derived_from<proto::INetObject> T>
    void Register(const ObjectConstructor& Contructor = []() { return new T(); });
    template < std::derived_from<proto::INetObject> T>
    void Unregister();


    template < std::derived_from<proto::INetObject> NetObject, std::derived_from<proto::INetProperty> PropType>
    bool RegisterPropertyListener(const proto::INetProperty& Property, const PropertyListenerWithT<PropType>& Listener);

    void Unregister(const uint32_t ObjectClassId);

    uint32_t NetObjectsCount() const;

    HERMOD_API NetObjectType Instantiate(const uint32_t ObjectClassId) const;
    template < typename... Args >
    NetObjectType Instantiate(const uint32_t ObjectClassId, Args&&... InArgs) const;

    void ReplicateObjects(std::vector < std::shared_ptr < IConnection >> Connections);

    HERMOD_API std::optional<PropertiesListenerContainer> GetPropertiesListeners(proto::INetObject& NetObject) const;


private:

    ObjectsConstructorContainer Factory;
    ObjectsListenerContainer ObjectListeners;
    std::vector<std::shared_ptr<proto::INetObject>> NetObjects;
};

#include "NetObjectManager.inl"