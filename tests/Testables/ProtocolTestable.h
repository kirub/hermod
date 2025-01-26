#include <hermod/protocol/Protocol.h>

#include "../Mocks/MockSocket.h"

class ProtocolTestable
	: public Protocol
{

	bool ForcePacketLost = false;

public:
	ProtocolTestable(uint32_t ProtocolId)
		: Protocol(ProtocolId)
	{
	}

	void PacketLost(bool Enable)
	{
		ForcePacketLost = Enable;
	}
	void TriggerPacketLost(uint16_t PacketId)
	{
		Protocol::TriggerPacketLost(PacketId);
	}

	uint16_t GetLastSentPacketId() const
	{
		return GetLatestSequenceId(Local);
	}

	uint16_t GetLastReceivedtPacketId() const
	{
		return GetLatestSequenceId(Remote);
	}

private:

	virtual void AckPacket(const uint16_t InAckedPacket) override;
	virtual void CachePacket(uint16_t NewSequenceId, SequenceIdType InSeqType) override;
};