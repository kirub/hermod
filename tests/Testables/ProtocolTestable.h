#pragma once

#include <hermod/protocol/Protocol.h>

#include "../Mocks/MockSocket.h"

class ProtocolTestable
	: public Protocol
{
	bool ForcePacketLost = false;

public:
	ProtocolTestable(uint32_t ProtocolId);

	void PacketLost(bool Enable);
	void TriggerPacketLost(uint16_t PacketId);

	uint16_t GetLastSentPacketId() const;
	uint16_t GetLastReceivedtPacketId() const;

private:

	virtual void AckPacket(const uint16_t InAckedPacket) override;
	virtual void CachePacket(uint16_t NewSequenceId, SequenceIdType InSeqType) override;
};