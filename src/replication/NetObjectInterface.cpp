#include <hermod/replication/NetObjectInterface.h>
#include <hermod/replication/NetPropertyInterface.h>
#include <hermod/utilities/Hash.h>
#include <hermod/utilities/Types.h>
#include <hermod/replication/NetObjectManager.h>
#include <hermod/serialization/NetIdMapping.h>

namespace proto
{
	INetObject::INetObject()
		: INetObject(Object)
	{
	}
	INetObject::INetObject(ENetObjectType InNetObjectType)
		: Properties()
		, NetObjectType(InNetObjectType)
		, NetId(serialization::NetIdMapping::Get().GetOrAssignedNetId(this))
	{
	}

	NetObjectId INetObject::GetId() const
	{
		return NetId;
	}

	bool INetObject::IsDirty() const
	{	
		return IsDirtyFlag;
	}

	void INetObject::SetDirty(bool InDirtyFlag, bool SetAllProperty /*= false*/)
	{	
		IsDirtyFlag = InDirtyFlag;
		if (SetAllProperty)
		{
			for (utils::IIntrusiveElement<255>* Element : Properties)
			{
				if (INetProperty* Property = (INetProperty*)Element)
				{
					Property->SetDirty(InDirtyFlag);
				}
			}
		}
	}
	bool INetObject::SerializeImpl(serialization::IStream& Stream)
	{
		return true;
	}

	bool INetObject::Serialize(serialization::IStream& Stream, std::optional<NetObjectManager::PropertiesListenerContainer> Mapper)
	{
		bool HasError = false;
		if (Stream.IsWriting())
		{
			uint32_t NetObjectClassId = GetClassId();
			HasError = !Stream.Serialize(NetObjectClassId);
		}
		
		HasError = !(!HasError && SerializeProperties(Stream, Mapper));

		if (!HasError && Stream.IsReading())
		{
			OnReceived();
		}

		return !HasError;
	}

	bool INetObject::SerializeProperties(serialization::IStream& Stream, std::optional<NetObjectManager::PropertiesListenerContainer> Mapper)
	{
		bool HasError = false;

		if (NetObjectType == Object)
		{
			Stream.Serialize(NetId);
		}

		for (utils::IIntrusiveElement<255>* Element : Properties)
		{
			if (INetProperty* Property = (INetProperty*)Element)
			{
				HasError = !Property->Serialize(Stream);
				if (HasError)
				{
					break;
				}

				if (Mapper)
				{
					NetObjectManager::PropertiesListenerContainer::const_iterator ListenerFound = Mapper->find(static_cast<uint8_t>(Property->GetIndex()));
					if (ListenerFound != Mapper->end())
					{
						ListenerFound->second(*Property);
					}
				}
			}
		}
		HasError &= !SerializeImpl(Stream);

		return !HasError;
	}

	void INetObject::AddProperty(INetProperty& Property)
	{
		assert(!Property.HasIndex());
		Properties.Add(Property);
	}
	void INetObject::RemoveProperty(INetProperty& Property)
	{
		assert(Property.HasIndex());
		Properties.Remove(Property);
	}
}
