#pragma once

#include "ConnectionInterface.h"
#include "hermod/protocol/Protocol.h"
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

class Connection
	: public IConnection
{
public:
	using ProtocolPtr = std::shared_ptr<IProtocol>;

	using IConnection::Send;

	HERMOD_API Connection(ProtocolPtr InProtocol, TimeMs InConnectionTimeoutMs);
	HERMOD_API Connection(ProtocolPtr InProtocol, Address InRemoteEndpoint, TimeMs InConnectionTimeoutMs);
	HERMOD_API virtual ~Connection();

	HERMOD_API virtual bool Send(proto::NetObjectPtr InNetObject) override;
	HERMOD_API virtual proto::NetObjectPtr Receive();
	HERMOD_API virtual bool OnPacketReceived(serialization::ReadStream& InStream) override;
	HERMOD_API virtual bool OnPacketSent(serialization::WriteStream& InStream, int32_t& MessageIncludedCount) override;

	HERMOD_API virtual proto::NetObjectQueue256& GetNetObjectQueue(ObjectQueueType InQueueType) override;
	HERMOD_API virtual class Address const & GetRemoteEndpoint() const override;

	HERMOD_API bool IsConnected() const;
	HERMOD_API bool IsClient() const;
	HERMOD_API bool IsServer() const;

	HERMOD_API void Update(TimeMs timeDelta);

protected:
	void OnMessageReceived(serialization::ReadStream& Stream, uint8_t NetObjectOrderId = 255, uint8_t NetObjectIdSpaceCount = 1);
	void AckPacketSent(uint16_t InPacketId);
	void Resend(uint16_t InPacketId);
	bool Flush();

	uint8_t GetPacketIdx(uint16_t SequenceId) const;

	static const int PacketSentHistorySize = 255;

	const TimeMs ConnectionTimeoutSec;
	TimeMs LastPacketReceiveTimeout;
	bool IsServerConnection;
	serialization::ReadStream Reader;
	serialization::WriteStream Writer;
	std::shared_ptr<IProtocol> Protocol;
	Address RemoteEndpoint;
	proto::FragmentHandler Fragments;
	proto::NetObjectQueue256 NetObjectQueues[ObjectQueueType::Count];
	std::vector<uint8_t> MessageIds[PacketSentHistorySize];
};