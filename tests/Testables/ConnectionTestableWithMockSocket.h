#pragma once
#include <hermod/replication/NetObjectInterface.h>
#include <hermod/protocol/Connection.h>

#include "../Mocks/MockSocket.h"
#include "ProtocolTestable.h"

class ConnectionTestableWithMockSocket
	: public Connection<MockSocket, ProtocolTestable>
{
public:
	using Super = Connection<MockSocket, ProtocolTestable>;
	static const int InBoundPort = 30000;

	ProtocolTestable* ProtoTest;

	enum EPacketLostBehaviour
	{
		None,
		DropInitial,	// won't drop resend
		DropAll,		// drop everything including resends
	};

	ConnectionTestableWithMockSocket(TimeMs InConnectionTimeout = 10000)
		: Super(InBoundPort, InConnectionTimeout)
		, DropStrategy(None)
		, ProtoTest(nullptr)
		, LastSentPacketId(Protocol::InvalidSequenceId)
	{

		ProtoTest = (ProtocolTestable*)MyProtocol.get();
	}

	uint16_t GetLastPacketId() const
	{
		return LastSentPacketId;
	}

	void RemoteDropPackets(EPacketLostBehaviour InBehaviour)
	{
		DropStrategy = InBehaviour;
	}

	void SimulateNextPacketLost()
	{
		RemoteDropPackets(DropInitial);
	}

private:
	EPacketLostBehaviour DropStrategy;
	uint16_t LastSentPacketId;
};