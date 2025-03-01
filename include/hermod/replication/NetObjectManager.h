#pragma once

#include <hermod/Platform/Platform.h>
#include <hermod/utilities/Types.h>
#include <functional>
#include <map>
#include <memory>
#include <optional>

class IConnection;

namespace proto
{
    class INetProperty;
    class INetObject;
}

class NetObjectManager
{
    NetObjectManager() = default;
public:
    using NetObjectType = std::shared_ptr<proto::INetObject>;
    using ObjectConstructor = std::function<proto::INetObject*()>;
    template < typename T>
    using PropertyListenerWithT = std::function<void(const T&)>;
    using PropertyListener = std::function<void(const proto::INetProperty&)>;
    using ObjectsConstructorContainer = std::map< uint32_t, ObjectConstructor>;
    using PropertiesListenerContainer = std::map< uint8_t, PropertyListener>;
    using ObjectsListenerContainer = std::map< uint32_t, PropertiesListenerContainer>;

    HERMOD_API static NetObjectManager& Get();

    template < typename T>
    typename typename enable_if<is_derived_from<T, proto::INetObject>::Value>::Type Register(const ObjectConstructor& Contructor = []() { return new T(); });
    template < typename T>
    typename enable_if<is_derived_from<T, proto::INetObject>::Value>::Type Unregister();


    template < typename NetObject, typename PropType, typename enable_if<is_derived_from<NetObject, proto::INetObject>::Value && is_derived_from<PropType, proto::INetProperty>::Value>::Type >
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