
#include <hermod/replication/NetPropertyInterface.h>

template < std::derived_from<proto::INetObject> T>
void NetObjectManager::Register(const ObjectConstructor& Contructor /*= []() { return new T(); }*/)
{
    Factory.insert({ T::NetObjectId::value, Contructor });
}
template < std::derived_from<proto::INetObject> T>
void NetObjectManager::Unregister()
{
    Factory.erase(T::NetObjectId::value);
}


template < std::derived_from<proto::INetObject> NetObject, std::derived_from<proto::INetProperty> PropType>
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