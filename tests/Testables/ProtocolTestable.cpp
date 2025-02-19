#include "ProtocolTestable.h"


ProtocolTestable::ProtocolTestable(uint32_t ProtocolId)
	: Protocol(ProtocolId)
{
}

void ProtocolTestable::AckPacket(const uint16_t InAckedPacket)
{
	Protocol::AckPacket(InAckedPacket);
}
void ProtocolTestable::CachePacket(uint16_t NewSequenceId, SequenceIdType InSeqType)
{
	Protocol::CachePacket(NewSequenceId, InSeqType);

	if (InSeqType == Local && ForcePacketLost)
	{
		TriggerPacketLost(NewSequenceId);
	}
}

void ProtocolTestable::PacketLost(bool Enable)
{
	ForcePacketLost = Enable;
}
void ProtocolTestable::TriggerPacketLost(uint16_t PacketId)
{
	Protocol::TriggerPacketLost(PacketId);
}

uint16_t ProtocolTestable::GetLastSentPacketId() const
{
	return GetLatestSequenceId(Local);
}

uint16_t ProtocolTestable::GetLastReceivedtPacketId() const
{
	return GetLatestSequenceId(Remote);
}