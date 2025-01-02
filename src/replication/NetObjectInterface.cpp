#include <hermod/replication/NetObjectInterface.h>
#include <hermod/utilities/Hash.h>
#include <hermod/utilities/Types.h>
#include <hermod/replication/NetObjectManager.h>

namespace proto
{
	INetProperty::INetProperty()
		: Dirty()
	{
	}

	INetProperty::INetProperty(INetObject& InPacket)
		: Dirty(false)
	{
		InPacket.AddProperty(*this);
		Dirty = true;
	}

	bool INetProperty::IsDirty() const
	{
		return Dirty;
	}

	void INetProperty::SetDirty(bool InDirtiness)
	{
		Dirty = InDirtiness;
	}

	bool INetProperty::Serialize(serialization::IStream& Stream)
	{
		bool HasError = false;
		HasError = !Stream.Serialize(Dirty, serialization::NetPropertySettings<bool>());

		if (!HasError && Dirty)
		{
			HasError = !SerializeImpl(Stream);
		}

		return HasError;
	}

	INetObject::INetObject()
		: INetObject(Object)
	{
	}
	INetObject::INetObject(ENetObjectType InNetObjectType)
		: NetObjectType(*this, InNetObjectType)
	{
	}

	bool INetObject::SerializeImpl(serialization::IStream& Stream)
	{
		return true;
	}

	bool INetObject::SerializeProperties(serialization::IStream& Stream, std::optional<NetObjectManager::PropertiesListenerContainer> Mapper)
	{
		for (INetProperty* Property : Properties)
		{
			if (Property && Property->IsDirty())
			{
				if (!Property->Serialize(Stream))
				{
					break;
				}

				Property->SetDirty(false);

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

		return false;
	}

	bool INetObject::Serialize(serialization::IStream& Stream, std::optional<NetObjectManager::PropertiesListenerContainer> Mapper)
	{
		bool HasError = false;
		if (Stream.IsWriting())
		{
			uint32_t NetObjectClassId = GetClassId();
			HasError = !Stream.Serialize(NetObjectClassId);
		}
		HasError &= !SerializeProperties(Stream, Mapper);
		HasError &= !SerializeImpl(Stream);

		if (!HasError && Stream.IsReading())
		{
			OnReceived();
		}

		return HasError;
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
