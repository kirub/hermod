#include "ProtocolTestable.h"

void ProtocolTestable::AckPacket(const uint16_t InAckedPacket)
{
	Protocol::AckPacket(InAckedPacket);
}
void ProtocolTestable::CachePacket(uint16_t NewSequenceId, SequenceIdType InSeqType)
{
	Protocol::CachePacket(NewSequenceId, InSeqType);

	if (InSeqType == Local && ForcePacketLost)
	{
		OnPacketLostCallback(NewSequenceId);
	}
}