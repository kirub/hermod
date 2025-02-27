
#include <hermod/replication/NetPropertyInterface.h>

template < typename T>
typename enable_if<is_derived_from<T, proto::INetObject>::Value>::Type NetObjectManager::Register(const ObjectConstructor& Contructor /*= []() { return new T(); }*/)
{
    Factory.insert({ T::NetObjectId::value, Contructor });
}
template < typename T>
typename enable_if<is_derived_from<T, proto::INetObject>::Value>::Type NetObjectManager::Unregister()
{
    Factory.erase(T::NetObjectId::value);
}


template < typename NetObject, typename PropType, typename enable_if<is_derived_from<NetObject, proto::INetObject>::Value&& is_derived_from<PropType, proto::INetProperty>::Value>::Type >
bool NetObjectManager::RegisterPropertyListener(const proto::INetProperty& Property, const PropertyListenerWithT<PropType>& Listener)
{
    return ObjectListeners.insert({ NetObject::NetObjectId::value, { Property.GetIndex(), Listener} }).second;
}

template < typename... Args >
NetObjectManager::NetObjectType NetObjectManager::Instantiate(const uint32_t ObjectClassId, Args&&... InArgs) const
{
    ObjectsConstructorContainer::const_iterator itFound = Factory.find(ObjectClassId);
    if (itFound == Factory.end())
    {
        return nullptr;
    }

    return std::make_shared(itFound->second(std::forward<Args>(InArgs)...));
}