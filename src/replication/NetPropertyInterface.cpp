#include <hermod/replication/NetPropertyInterface.h>
#include <hermod/replication/NetObjectInterface.h>
#include <hermod/serialization/Stream.h>

namespace proto
{
	INetProperty::INetProperty(INetObject& InParent)
		: Parent(InParent)
		, Dirty(true)
	{
	}

	bool INetProperty::IsDirty() const
	{
		return Dirty;
	}

	void INetProperty::SetDirty(bool InDirtiness, bool NotifyParent /*= false*/)
	{
		Dirty = InDirtiness;
		if (NotifyParent)
		{
			Parent.SetDirty(InDirtiness);
		}
	}

	bool INetProperty::Serialize(serialization::IStream& Stream)
	{
		bool HasError = false;
		HasError = !Stream.Serialize(Dirty, serialization::NetPropertySettings<bool>());

		if (!HasError && Dirty)
		{
			HasError = !SerializeImpl(Stream);
		}		

		return !HasError;
	}
}
