#include <hermod/serialization/NetIdMapping.h>
#include <hermod/replication/NetObjectInterface.h>

namespace serialization
{
    NetIdMapping& NetIdMapping::Get() {
        static NetIdMapping instance;
        return instance;
    }

    void NetIdMapping::Clear()
    {
        ObjectToIdMap.clear();
        IdToObjectMap.clear();
    }

    NetObjectId NetIdMapping::GetOrAssignedNetId(ObjectInstanceToIdContainer::key_type Value)
    {
        auto [ItElement, Inserted] = ObjectToIdMap.insert({ Value, InvalidNetObjectId });
        assert(!Inserted || !IdToObjectMap.contains(ItElement->second));
        if (Inserted)
        {
            ItElement->second = PTR_TO_ID(Value);
            IdToObjectMap[ItElement->second] = Value;
        }
        return ItElement->second;
    }

    NetIdMapping::NetObjectType NetIdMapping::GetObjectFromNetId(ObjectIdToInstanceContainer::key_type Value) const
    {
        if (ObjectIdToInstanceContainer::const_iterator ItElement = IdToObjectMap.find(Value); ItElement != IdToObjectMap.end())
        {
            assert(ObjectToIdMap.contains(ItElement->second));
            return ItElement->second;
        }

        return nullptr;
    }
}