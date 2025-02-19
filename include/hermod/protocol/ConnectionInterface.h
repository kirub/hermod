#pragma once

#include <hermod/platform/Platform.h>
#include <hermod/utilities/Types.h>
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

class HERMOD_API IConnection
{

public:
	enum ObjectQueueType
	{
		SendQueue,
		ReceiveQueue,
		Count
	};

	using OnReceiveDataFunctor = void(*)(unsigned char*, const int);
	using OnReceiveObjectFunctor = std::function<void(proto::INetObject&)>;

	enum Error
	{
		None,
		DisconnectionError,
		InvalidRemoteAddress,
		InvalidHeader
	};

	IConnection()
		: ReceiveDataCallback(nullptr)
		, ReceiveObjectCallback(nullptr)
		, NeedsAck(false)
	{

	}
	virtual ~IConnection()
	{ }

	template < std::derived_from<proto::INetObject> T>
	bool Send(T InNetObject)
	{
		return Send(std::static_pointer_cast<proto::INetObject>(std::make_shared<T>(std::move(InNetObject))));
	}
	virtual bool Send(proto::NetObjectPtr InNetObject) = 0;
	virtual bool Send(serialization::WriteStream& Packet) = 0;
	virtual bool Send(unsigned char* Data, std::size_t Len) = 0;
	virtual proto::NetObjectPtr Receive() = 0;
	virtual const unsigned char* GetData() = 0;
	virtual proto::NetObjectQueue256& GetNetObjectQueue(ObjectQueueType InQueueType) = 0;
	virtual void OnPacketReceived(serialization::ReadStream& Stream) = 0;

	virtual bool IsConnected() const = 0;
	virtual bool IsClient() const = 0;
	virtual bool IsServer() const = 0;

	virtual class Address const& GetRemoteEndpoint() const = 0;

	virtual Error Update(TimeMs timeDelta) = 0;

	void OnReceiveData(OnReceiveDataFunctor InReceiveDataCallback)
	{
		ReceiveDataCallback = InReceiveDataCallback;
	}
	void OnReceiveObject(OnReceiveObjectFunctor InReceiveObjectCallback)
	{
		ReceiveObjectCallback = InReceiveObjectCallback;
	}

	void SetNeedsAck()
	{
		NeedsAck.store(true);
	}
	bool GetNeedsAckAndReset()
	{
		return NeedsAck.exchange(false);
	}
protected:

	OnReceiveDataFunctor ReceiveDataCallback;
	OnReceiveObjectFunctor ReceiveObjectCallback;
	std::atomic_bool NeedsAck;
};