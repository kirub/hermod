#include <hermod/protocol/Protocol.h>

#include "hermod/serialization/Stream.h"
#include "hermod/serialization/WriteStream.h"
#include "hermod/utilities/Utils.h"

#include <limits>

Protocol::Protocol(unsigned int InId)
	: Id(InId)
	, NextRemoteSequenceIdx(0)
	, NextLocalSequenceIdx(0)
	, NotAckedPackets()
	, OnPacketAckedCallback(nullptr)
	, OnPacketLostCallback(nullptr)
	, CurrentRTT(0)
{
	memset(RemoteSequenceIdHistory, InvalidSequenceId, sizeof(uint16_t) * HistorySize);
	memset(LocalSequenceIdHistory, InvalidSequenceId, sizeof(uint16_t) * HistorySize);
	memset(NotAckedPackets, InvalidSequenceId, sizeof(uint16_t) * HistorySize);
}

bool Protocol::WriteHeader(unsigned char* Data, int Len)
{
	if (!WriteProtocolId(Data, Len))
	{
		return false;
	}

	if (!WriteSequenceId(Data, Len))
	{
		return false;
	}

	if (!WriteAck(Data, Len))
	{
		return false;
	}

	return true;
}

bool Protocol::Serialize(serialization::IStream& InStream)
{
	uint32_t ProtocolId = 0;
	if (InStream.IsWriting())
	{
		ProtocolId = Id;
	}

	if (!InStream.Serialize(ProtocolId))
	{
		printf(__FUNCTION__ ": Unable to %s ProtocolId\n", InStream.Operation().c_str() );
		return false;
	}

	if (InStream.IsReading() && ProtocolId != Id)
	{
		printf(__FUNCTION__ ": Invalid Protocol Id\n");
		return false;
	}

	uint16_t SequenceId = 0;
	if (InStream.IsWriting())
	{
		SequenceId = GetNextSequenceId(Local);
	}

	if (!InStream.Serialize(SequenceId))
	{
		printf(__FUNCTION__ ": Unable to %s SequenceId\n", InStream.Operation().c_str());
		return false;
	}

	if (InStream.IsReading())
	{
		if (SequenceId == InvalidSequenceId || !utils::sequence_greater_than(SequenceId, GetLatestSequenceId(Remote)))
		{
			printf(__FUNCTION__ ": Invalid or old packet\n");
			return false;
		}

		CachePacket(SequenceId, Remote);
	}

	uint16_t Ack = 0;
	if (InStream.IsWriting())
	{
		Ack = GetLatestSequenceId(Remote);
	}

	if (!InStream.Serialize(Ack))
	{
		printf(__FUNCTION__ ": Unable to %s Ack\n", InStream.Operation().c_str());
		return false;
	}

	const bool AckProvided = Ack != InvalidSequenceId;

	uint32_t AckBitfield = 0;

	if (InStream.IsWriting())
	{
		if (AckProvided)
		{
			AckBitfield = ComputeAckBitfield(Ack);
		}
	}

	if (!InStream.Serialize(AckBitfield))
	{
		printf(__FUNCTION__ ": Unable to %s AckBitfield\n", InStream.Operation().c_str());
		return false;
	}

	if (InStream.IsReading())
	{
		if (AckProvided)
		{
			AckPacket(Ack);

			for (int BitIdx = 0; BitIdx < HistorySize - 1; ++BitIdx)
			{
				if (AckBitfield & (1 << BitIdx))
				{
					uint16_t SequenceId = Ack - BitIdx - 1;
					AckPacket(SequenceId);
				}
			}
		}
	}
	assert(InStream.Align(32));

	return true;
}

bool Protocol::CheckHeader(const unsigned char*& Data, int& Len)
{
	if (Len < Size())
	{
		printf(__FUNCTION__ ": Invalid header\n");
		return false;
	}

	UINT32 RemoteProtocolId = 0;
	if (!ReadProtocolId(RemoteProtocolId, Data, Len) || RemoteProtocolId != Id)
	{
		printf(__FUNCTION__ ": Invalid Protocol Id\n");
		return false;
	}

	uint16_t RemoteSequenceId = InvalidSequenceId;
	if (!ReadSequenceId(RemoteSequenceId, Data, Len) || RemoteSequenceId == InvalidSequenceId || !utils::sequence_greater_than(RemoteSequenceId, GetLatestSequenceId(Remote)))
	{
		printf(__FUNCTION__ ": Invalid or old packet\n");
		return false;
	}

	CachePacket(RemoteSequenceId, Remote);

	if (!ReadAndAck(Data, Len))
	{
		printf(__FUNCTION__ ": Invalid acks\n");
		return false;
	}

	return true;
}


void Protocol::OnPacketAcked(const OnPacketAckedCallbackType& Callback)
{
	OnPacketAckedCallback = Callback;
}

uint16_t Protocol::OnPacketSent(serialization::WriteStream InStream)
{
	uint32_t* BufferAtSequenceIdOffset=((uint32_t*)InStream.GetData()) + 1;
	uint16_t PacketSentSequenceId = (uint16_t)(ntohl(*BufferAtSequenceIdOffset) & 0xFFFF);
	return OnPacketSent(PacketSentSequenceId);
}

uint16_t Protocol::OnPacketSent(unsigned char* Buffer, int Len)
{
	uint16_t PacketSentSequenceId = InvalidSequenceId;
	const unsigned char* BufferAtPacketIdLocation = Buffer + sizeof(UINT32);
	if (ReadSequenceId(PacketSentSequenceId, BufferAtPacketIdLocation, Len) && PacketSentSequenceId != InvalidSequenceId)
	{
		return OnPacketSent(PacketSentSequenceId);
	}

	return InvalidSequenceId;
}
uint16_t Protocol::OnPacketSent(const uint16_t PacketSentSequenceId)
{
	CachePacket(PacketSentSequenceId, Local);
	return PacketSentSequenceId;
}
void Protocol::OnPacketLost(const OnPacketLostCallbackType& Callback)
{
	OnPacketLostCallback = Callback;
}

bool Protocol::ReadProtocolId(UINT32& ProtocolId, const unsigned char*& Data, int& Len) const
{
	const std::size_t SizeOfProtocolId = sizeof(unsigned int);
	if (Len < SizeOfProtocolId)
	{
		printf(__FUNCTION__ ": buffer too small\n");
		return false;
	}

	ProtocolId = UINT32_MAX;
	memcpy(&ProtocolId, Data, sizeof(unsigned int));
	ProtocolId = ntohl(ProtocolId);

	Data	+= SizeOfProtocolId;
	Len		-= SizeOfProtocolId;

	return true;
}

 bool Protocol::ReadSequenceId(uint16_t& RemoteSequenceId, const unsigned char*& Data, int& Len) const
{
	const std::size_t SizeOfSequenceId = sizeof(uint16_t);
	if (Len < SizeOfSequenceId)
	{
		printf(__FUNCTION__ ": buffer too small\n");
		return false;
	}

	RemoteSequenceId = InvalidSequenceId;
	memcpy(&RemoteSequenceId, Data, SizeOfSequenceId);
	RemoteSequenceId = ntohs(RemoteSequenceId);

	Data	+= SizeOfSequenceId;
	Len		-= SizeOfSequenceId;

	return true;
}

bool Protocol::ReadAck(uint16_t& Ack, const unsigned char*& Data, int& Len) const
{
	const std::size_t SizeOfAck = sizeof(uint16_t);
	if (Len < SizeOfAck)
	{
		printf(__FUNCTION__ ": buffer too small\n");
		return false;
	}

	Ack = InvalidSequenceId;
	memcpy(&Ack, Data, SizeOfAck);
	Ack = ntohs(Ack);

	Data	+= SizeOfAck;
	Len		-= SizeOfAck;

	return true;
}

bool Protocol::ReadAckBitfield(UINT32& AckBitfield, const unsigned char*& Data, int& Len, bool Skip) const
{
	const std::size_t SizeOfAckBitfield = sizeof(UINT32);
	if (Len < SizeOfAckBitfield)
	{
		printf(__FUNCTION__ ": buffer too small\n");
		return false;
	}

	AckBitfield = UINT32_MAX;
	memcpy(&AckBitfield, Data, SizeOfAckBitfield);
	AckBitfield = ntohl(AckBitfield);

	Data	+= SizeOfAckBitfield;
	Len		-= SizeOfAckBitfield;

	return true;
}

UINT8 Protocol::GetLatestSequenceIdx(SequenceIdType InSeqIdType) const
{
	UINT8 SequenceIdIdx = (InSeqIdType == Local) ? NextLocalSequenceIdx : NextRemoteSequenceIdx;
	return SequenceIdIdx - 1 < 0 ? HistorySize - 1 : SequenceIdIdx - 1;;
}

UINT8 Protocol::GetNextSequenceIdx(SequenceIdType InSeqIdType) const
{
	UINT8 SequenceIdIdx = (InSeqIdType == Local) ? NextLocalSequenceIdx : NextRemoteSequenceIdx;
	return (SequenceIdIdx + 1) % HistorySize;
}

uint16_t Protocol::GetNextSequenceId(SequenceIdType InSeqIdType) const
{
	uint16_t NextSequenceId = GetLatestSequenceId(InSeqIdType) + 1;
	return utils::sequence_modulo(NextSequenceId, std::numeric_limits<uint16_t>::max() - 1); // - 1 for reserved std::numeric_limits<uint16_t>::max() being invalid
}

uint16_t Protocol::GetLatestSequenceId(SequenceIdType InSeqIdType) const
{
	UINT8 SequenceIdIdx = GetLatestSequenceIdx(InSeqIdType);
	const uint16_t* History = (InSeqIdType == Local) ? LocalSequenceIdHistory : RemoteSequenceIdHistory;
	return History[SequenceIdIdx];
}

bool Protocol::WriteProtocolId(unsigned char*& Data, int& Len) const
{
	constexpr std::size_t ProtocolIdLen = sizeof(unsigned int);
	if (Len < ProtocolIdLen)
	{
		printf(__FUNCTION__ ": buffer too small\n");
		return false;
	}

	unsigned int IdTemp = htonl(Id);
	memcpy(Data, &IdTemp, ProtocolIdLen);
	Data += ProtocolIdLen;
	Len -= ProtocolIdLen;

	return true;
}

bool Protocol::WriteSequenceId(unsigned char*& Data, int& Len)
{
	constexpr std::size_t LocalSequenceIdLen = sizeof(uint16_t);
	if (Len < LocalSequenceIdLen)
	{
		printf(__FUNCTION__ ": buffer too small\n");
		return false;
	}

	uint16_t LocalSequenceId = GetNextSequenceId(Local);
	LocalSequenceId = htons(LocalSequenceId);
	memcpy(Data, &LocalSequenceId, LocalSequenceIdLen);
	Data += LocalSequenceIdLen;
	Len -= LocalSequenceIdLen;

	return true;
}

bool Protocol::WriteAck(unsigned char*& Data, int& Len) const
{
	constexpr std::size_t AckLen = sizeof(uint16_t);
	constexpr std::size_t AckBitfieldLen = HistorySize / 8;
	if (Len < (AckLen + AckBitfieldLen))
	{
		printf(__FUNCTION__ ": buffer too small\n");
		return false;
	}

	// write remote sequence id after protocol id
	uint16_t LastRemoteSequenceId = GetLatestSequenceId(Remote);

	uint16_t LastRemoteSequenceIdNet = htons(LastRemoteSequenceId);
	memcpy(Data, &LastRemoteSequenceIdNet, AckLen);

	if (LastRemoteSequenceId != InvalidSequenceId)
	{
		UINT32 AckBitfield = ComputeAckBitfield(LastRemoteSequenceId);
		AckBitfield = htonl(AckBitfield);
		memcpy(Data + AckLen, &AckBitfield, AckBitfieldLen);
	}
	Data += (AckLen + AckBitfieldLen);
	Len -= (AckLen + AckBitfieldLen);

	return true;
}

UINT32 Protocol::ComputeAckBitfield(const uint16_t ReferenceSequenceId) const
{
	UINT32 AckBitfield = 0;
	uint16_t LastReceiveSequenceIdx = GetLatestSequenceIdx(Remote);
	for (int BitIdx = 0; BitIdx < HistorySize-1; ++BitIdx)
	{
		uint16_t RemoteSequenceIdToFind = utils::sequence_modulo(ReferenceSequenceId - (BitIdx+1), std::numeric_limits<uint16_t>::max() - 1);
		constexpr int GoingBackward = -1;
		if (FindPacket(RemoteSequenceIdToFind, Remote, (UINT8)utils::sequence_modulo(LastReceiveSequenceIdx - (BitIdx + 1), HistorySize - 1), GoingBackward) != InvalidSequenceIdx)
		{
			AckBitfield |= 1 << BitIdx;
		}
	}

	return AckBitfield;
}


UINT32 Protocol::ComputeAckBitfield2(const uint16_t ReferenceSequenceId) const
{
	UINT32 AckBitfield = 0;
	for (int BitIdx = 0; BitIdx < HistorySize-1; ++BitIdx)
	{
		uint16_t RemoteSequenceIdToFind = ReferenceSequenceId - (BitIdx + 1);
		constexpr int GoingBackward = -1;
		if (CheckPacket(RemoteSequenceIdToFind, Remote))
		{
			AckBitfield = 1 << BitIdx;
		}
	}

	return AckBitfield;
}


UINT8 Protocol::FindPacket(const uint16_t InPacketId, SequenceIdType InSeqType, const UINT8 StartIdx /*= 0*/, const int Increment /*= 1*/) const
{
	const uint16_t* InHistory = InSeqType == Local ? LocalSequenceIdHistory : RemoteSequenceIdHistory;
	UINT8 Idx = StartIdx;
	for (int Iter = 0; Iter < HistorySize-1; ++Iter, Idx = (UINT8)utils::sequence_modulo(Idx + Increment, HistorySize - 1) )
	{
		if (InHistory[Idx] == InPacketId)
		{
			return Idx;
		}
	}

	return InvalidSequenceIdx;
}


bool Protocol::CheckPacket(const uint16_t InSequenceId, SequenceIdType InSeqType) const
{
	const uint16_t* InHistory = InSeqType == Local ? LocalSequenceIdHistory : RemoteSequenceIdHistory;

	const int Index = InSequenceId % HistorySize;
	return InHistory[Index] == InSequenceId;
}

void Protocol::AckPacket(const uint16_t InAckedPacket)
{
	const int Index = InAckedPacket % HistorySize;
	ReportRTT(NotAckedPackets[Index].RTT());
	NotAckedPackets[Index].Reset();

	if (OnPacketAckedCallback)
	{
		OnPacketAckedCallback(InAckedPacket);
	}
}

void Protocol::ReportRTT(const int64_t InRTT)
{
	if (InRTT > MaxRTT)
	{
		// Discard out of bounds values
		return;
	}

	const int64_t RTTInc = (int64_t)((InRTT - CurrentRTT) * (RTTIncFactor / 100.0f));
	CurrentRTT += RTTInc;
}


const int64_t Protocol::GetRTT() const
{
	return CurrentRTT;
}

bool Protocol::ReadAndAck(const unsigned char*& Data, int& Len)
{
	uint16_t Ack = InvalidSequenceId;
	if (!ReadAck(Ack, Data, Len))
	{
		return false;
	}

	const bool NoAckProvided = Ack == InvalidSequenceId;

	UINT32 AckBitfield = 0;
	if (!ReadAckBitfield(AckBitfield, Data, Len))
	{
		printf(__FUNCTION__ ": Invalid Ack bitfield\n");
		return false;
	}

	if (NoAckProvided)
	{
		return true;
	}

	AckPacket(Ack);
	for (int BitIdx = 0; BitIdx < HistorySize - 1; ++BitIdx)
	{
		if (AckBitfield & (1 << BitIdx))
		{
			uint16_t SequenceId = Ack - BitIdx - 1;
			AckPacket(SequenceId);
		}
	}

	return true;
}

void Protocol::CachePacket(uint16_t NewSequenceId, SequenceIdType InSeqType)
{
	UINT8& NextSequenceIdx = InSeqType == Local ? NextLocalSequenceIdx : NextRemoteSequenceIdx;
	uint16_t* History = InSeqType == Local ? LocalSequenceIdHistory : RemoteSequenceIdHistory;

	if (InSeqType == Local)
	{
		const int IndexJustSentPacketInAckedPacket = NewSequenceId % HistorySize;
		UINT16 EvictedSequenceId = NotAckedPackets[IndexJustSentPacketInAckedPacket].SequenceId;
		// Means there was a packet here before that we are evicting from history
		if (EvictedSequenceId != InvalidSequenceId)
		{
			TriggerPacketLost(EvictedSequenceId);
		}
		NotAckedPackets[IndexJustSentPacketInAckedPacket] = NewSequenceId;
	}

	History[NextSequenceIdx] = NewSequenceId;
	NextSequenceIdx = GetNextSequenceIdx(InSeqType);
}

void Protocol::TriggerPacketLost(uint16_t PacketSequenceId) const
{
	printf(__FUNCTION__ ": Packet %u lost\n", PacketSequenceId);
	if (OnPacketLostCallback)
	{
		OnPacketLostCallback(PacketSequenceId);
	}
}

const int Protocol::Size() const
{
	return sizeof(unsigned int) + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(UINT32);
}