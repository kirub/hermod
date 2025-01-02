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

template < TSocket SocketType>
class Connection
	: public IConnection
{
public:

	Connection(unsigned short InboundPort, TimeMs InConnectionTimeoutMs);
	Connection(Address InRemoteEndpoint, unsigned short InboundPort, TimeMs InConnectionTimeoutMs);

	virtual bool Send(proto::INetObject& Packet);
	bool Send(unsigned char* Data, std::size_t Len);
	virtual const unsigned char* GetData();

	bool IsConnected() const;
	bool IsClient() const;
	bool IsServer() const;

	Error Update(TimeMs timeDelta);
private:

	bool Flush();
	void OnPacketReceived(serialization::ReadStream& Stream);

	static const int MaxStreamSize = 1024 * 64;

	const TimeMs ConnectionTimeoutSec;
	TimeMs LastPacketReceiveTimeout;
	bool IsServerConnection;
	serialization::ReadStream Reader;
	serialization::WriteStream Writer;
	Address RemoteEndpoint;
	std::unique_ptr<ISocket> Socket;
	std::unique_ptr<IProtocol> MyProtocol;
	proto::FragmentHandler Fragments;
};

#include "Connection.inl"