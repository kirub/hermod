#pragma once
#include <hermod/replication/NetObjectInterface.h>
#include <hermod/protocol/Connection.h>

#include "../Mocks/MockSocket.h"
#include "ProtocolTestable.h"

class ConnectionTestableWithMockSocket
	: public Connection
{
public:
	using Super = Connection;
	static const int ProtocolId = 1;

	enum EPacketLostBehaviour
	{
		None,
		DropInitial,	// won't drop resend
		DropAll,		// drop everything including resends
	};

	ConnectionTestableWithMockSocket(TimeMs InConnectionTimeout = 10000)
		: Super(std::make_shared<ProtocolTestable>(ProtocolId), InConnectionTimeout)
		, DropStrategy(None)
		, LastSentPacketId(Protocol::InvalidSequenceId)
	{
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