#pragma once

#include "ConnectionInterface.h"
#include "hermod/protocol/Protocol.h"
#include "hermod/protocol/Protocol2.h"
#include "hermod/protocol/FragmentHandler.h"
#include "hermod/protocol/NetObjectQueue.h"

#include <hermod/socket/UDPSocket.h>
#include <hermod/socket/Address.h>
#include <hermod/utilities/Utils.h>
#include <hermod/serialization/ReadStream.h>
#include <hermod/serialization/WriteStream.h>

#include <functional>

namespace proto
{
	class INetObject;
	class FragmentHandler;
}

template < TSocket SocketType, TProtocol ProtocolType>
class Connection
	: public IConnection
{
public:
	using IConnection::Send;

	Connection(unsigned short InboundPort, TimeMs InConnectionTimeoutMs);
	Connection(Address InRemoteEndpoint, unsigned short InboundPort, TimeMs InConnectionTimeoutMs);
	virtual ~Connection();

	virtual bool Send(proto::NetObjectPtr InNetObject) override;
	virtual bool Send(serialization::WriteStream& Stream) override;
	virtual proto::NetObjectPtr Receive();
	bool Send(unsigned char* Data, std::size_t Len);
	virtual const unsigned char* GetData() override;
	virtual proto::NetObjectQueue256& GetNetObjectQueue(ObjectQueueType InQueueType) override;
	virtual void OnPacketReceived(serialization::ReadStream& InStream) override;

	virtual class Address const & GetRemoteEndpoint() const override;

	bool IsConnected() const;
	bool IsClient() const;
	bool IsServer() const;

	Error Update(TimeMs timeDelta);

protected:
	void OnMessageReceived(serialization::ReadStream& Stream, uint8_t NetObjectOrderId = 255, uint8_t NetObjectIdSpaceCount = 1);
	void AckPacketSent(uint16_t InPacketId);
	void Resend(uint16_t InPacketId);
	bool Flush();

	const TimeMs ConnectionTimeoutSec;
	TimeMs LastPacketReceiveTimeout;
	bool IsServerConnection;
	serialization::ReadStream Reader;
	serialization::WriteStream Writer;
	Address RemoteEndpoint;
	std::unique_ptr<ISocket> Socket;
	std::unique_ptr<IProtocol> MyProtocol;
	proto::FragmentHandler Fragments;
	proto::NetObjectQueue256 NetObjectQueues[ObjectQueueType::Count];

	static const int PacketSentHistorySize = 64;
	serialization::WriteStream PacketsSent[PacketSentHistorySize];
};

#include "Connection.inl"