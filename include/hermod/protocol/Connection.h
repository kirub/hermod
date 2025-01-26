#pragma once

#include "ConnectionInterface.h"
#include "hermod/protocol/Protocol.h"
#include "hermod/protocol/Protocol2.h"
#include "hermod/protocol/FragmentHandler.h"

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

template < typename T> concept TSocket = std::derived_from<T, ISocket>;
template < typename T> concept TProtocol = std::derived_from<T, IProtocol>;

template < TSocket SocketType, TProtocol ProtocolType>
class Connection
	: public IConnection
{
public:	

	Connection(unsigned short InboundPort, TimeMs InConnectionTimeoutMs);
	Connection(Address InRemoteEndpoint, unsigned short InboundPort, TimeMs InConnectionTimeoutMs);

	virtual bool Send(proto::INetObject& Packet, EReliability InReliability = Unreliable) override;
	virtual bool Send(serialization::WriteStream& Stream, EReliability InReliability = Unreliable, bool IsResend = false) override;
	bool Send(unsigned char* Data, std::size_t Len);
	virtual const unsigned char* GetData() override;

	bool IsConnected() const;
	bool IsClient() const;
	bool IsServer() const;

	Error Update(TimeMs timeDelta);

protected:
	void AckPacketSent(uint16_t InPacketId);
	void Resend(uint16_t InPacketId);
	bool Flush();
	uint16_t OnPacketSent(serialization::WriteStream& InStream, EReliability InReliability = Unreliable);
	void OnPacketReceived(serialization::ReadStream& Stream);

	const TimeMs ConnectionTimeoutSec;
	TimeMs LastPacketReceiveTimeout;
	bool IsServerConnection;
	serialization::ReadStream Reader;
	serialization::WriteStream Writer;
	Address RemoteEndpoint;
	std::unique_ptr<ISocket> Socket;
	std::unique_ptr<IProtocol> MyProtocol;
	proto::FragmentHandler Fragments;

	static const int PacketSentHistorySize = 64;
	serialization::WriteStream PacketsSent[PacketSentHistorySize];

#if WITH_TESTS
	uint16_t LastSentPacketId;
#endif
};

#include "Connection.inl"