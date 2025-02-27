#pragma once

#include <hermod/platform/Platform.h>
#include <hermod/utilities/Types.h>
#include <hermod/utilities/Callable.h>
#include <hermod/replication/NetObjectInterface.h>

#include <cstddef>

namespace serialization
{
	class ReadStream;
	class WriteStream;
}

namespace proto
{
	class INetObject;
}

class IConnection
{

public:
	enum ObjectQueueType
	{
		SendQueue,
		ReceiveQueue,
		Count
	};

	enum Error
	{
		None,
		DisconnectionError,
		InvalidRemoteAddress,
		InvalidHeader
	};

	HERMOD_API IConnection()
		: NeedsAck(false)
	{

	}
	HERMOD_API virtual ~IConnection()
	{ }

	template < typename T, typename enable_if<is_derived_from<T, proto::INetObject>::Value>::Type>
	bool Send(T InNetObject)
	{
		return Send(std::static_pointer_cast<proto::INetObject>(std::make_shared<T>(std::move(InNetObject))));
	}
	HERMOD_API virtual bool Send(proto::NetObjectPtr InNetObject) = 0;
	HERMOD_API virtual proto::NetObjectPtr Receive() = 0;
	virtual proto::NetObjectQueue256& GetNetObjectQueue(ObjectQueueType InQueueType) = 0;
	virtual bool OnPacketReceived(serialization::ReadStream& Stream) = 0;
	virtual bool OnPacketSent(serialization::WriteStream& InStream, int32_t& MessageIncludedCount) = 0;

	HERMOD_API virtual bool IsConnected() const = 0;
	HERMOD_API virtual bool IsClient() const = 0;
	HERMOD_API virtual bool IsServer() const = 0;

	HERMOD_API virtual class Address const& GetRemoteEndpoint() const = 0;

	virtual void Update(TimeMs timeDelta) = 0;

	void SetNeedsAck()
	{
		NeedsAck.store(true);
	}
	bool GetNeedsAckAndReset()
	{
		return NeedsAck.exchange(false);
	}

	void OnConnected(Callable<void()> InCallable)
	{
		OnConnectedFunc = InCallable;
	}
	void OnDisconnected(Callable<void()> InCallable)
	{
		OnDisconnectedFunc = InCallable;
	}
protected:

	std::atomic_bool NeedsAck;
	Callable<void()> OnConnectedFunc;
	Callable<void()> OnDisconnectedFunc;
};