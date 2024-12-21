#pragma once

#include "ConnectionInterface.h"
#include "hermod/protocol/Protocol.h"
#include "hermod/protocol/Protocol2.h"

#include <hermod/socket/UDPSocket.h>
#include <hermod/socket/Address.h>
#include <hermod/utilities/Utils.h>
#include <hermod/serialization/ReadStream.h>
#include <hermod/serialization/WriteStream.h>

#include <functional>

namespace proto
{
	class INetObject;
}

class Connection
	: public IConnection
{

public:

	Connection(unsigned short InboundPort, TimeMs InConnectionTimeoutMs);
	Connection(Address InRemoteEndpoint, unsigned short InboundPort, TimeMs InConnectionTimeoutMs);

	virtual bool Send(proto::INetObject& Packet);
	bool Send(unsigned char* Data, std::size_t Len);
	const unsigned char* GetData() const;

	bool IsConnected() const;
	bool IsClient() const;
	bool IsServer() const;



	Error Update(TimeMs timeDelta);

private:
	void OnPacketReceived(unsigned char* Data, const int Len);

	static const int MaxPacketSize = 1024;

	const TimeMs ConnectionTimeoutSec;
	TimeMs LastPacketReceiveTimeout;
	unsigned char ReceiveBuffer[MaxPacketSize];
	unsigned char SendBuffer[MaxPacketSize];
	bool IsServerConnection;
	serialization::ReadStream Reader;
	serialization::WriteStream Writer;
	Address RemoteEndpoint;
	UDPSocket Socket;
	std::unique_ptr<IProtocol> MyProtocol;
};