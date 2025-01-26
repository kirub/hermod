#include <hermod/replication/NetObjectInterface.h>
#include <hermod/protocol/Connection.h>

#include "../Mocks/MockSocket.h"
#include "ProtocolTestable.h"

class ConnectionTestable
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

	ConnectionTestable(TimeMs InConnectionTimeout = 10000)
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

	virtual bool Send(proto::INetObject& Packet, EReliability InReliability = Unreliable) override
	{
		return Super::Send(Packet, InReliability);
	}

	virtual bool Send(serialization::WriteStream& Stream, EReliability InReliability = Unreliable, bool IsResend = false) override
	{
		bool HasSent = false;
		switch (DropStrategy)
		{
			case DropAll:
			{
				LastSentPacketId = Super::OnPacketSent(Stream, InReliability);
			}
			break;
			case DropInitial:
			{
				if (!IsResend)
				{
					LastSentPacketId = Super::OnPacketSent(Stream, InReliability);
				}
				else if (!Super::Send(Stream, InReliability))
				{
					return false;
				}

				return true;
			}
			break;
			case None:
			{
				if (Super::Send(Stream, InReliability))
				{
					return true;
				}
			}
			break;
		}

		return false;
	}

private:
	EPacketLostBehaviour DropStrategy;
	uint16_t LastSentPacketId;
};