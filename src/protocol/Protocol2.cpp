#include "hermod/protocol/Protocol2.h"
#include "hermod/utilities/Utils.h"

#include <cassert>

Protocol2::Protocol2(unsigned int InId)
	: Id(InId)
	, LocalSequenceId(0)
	, LastRemoteSequenceIdx(InvalidSequenceIdx)
	, OnPacketAckedCallback(nullptr)
{
	memset(LastAckedPackets, InvalidSequenceId, sizeof(uint16_t) * HistorySize);
	memset(RemoteSequenceIdHistory, InvalidSequenceId, sizeof(uint16_t) * HistorySize);
	memset(LocalSequenceIdHistory, InvalidSequenceId, sizeof(uint16_t) * HistorySize);
}

bool Protocol2::WriteHeader(unsigned char* Data, int Len)
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

bool Protocol2::CheckHeader(const unsigned char*& Data, int& Len)
{
	if (Len < Size())
	{
		printf(__FUNCTION__ ": Invalid header\n");
		return false;
	}

	UINT32 RemoteProtocolId = 0;
	if (!ReadProtocolId(RemoteProtocolId, Data, Len) || RemoteProtocolId != Id)
	{
		printf(__FUNCTION__ ": Invalid Protocol2 Id\n");
		return false;
	}

	uint16_t RemoteSequenceId = InvalidSequenceId;
	if (!ReadSequenceId(RemoteSequenceId, Data, Len) || RemoteSequenceId == InvalidSequenceId || !utils::sequence_greater_than(RemoteSequenceId, GetLastSequenceId(Remote)))
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


void Protocol2::OnPacketAcked(const OnPacketAckedCallbackType& Callback)
{
	OnPacketAckedCallback = Callback;
}

void Protocol2::OnPacketSent(const unsigned char* Buffer, int Len)
{
	uint16_t PacketSentSequenceId = InvalidSequenceId;
	const unsigned char* BufferAtPacketIdLocation = Buffer + sizeof(UINT32);
	if (ReadSequenceId(PacketSentSequenceId, BufferAtPacketIdLocation, Len) && PacketSentSequenceId != InvalidSequenceId)
	{
		CachePacket(PacketSentSequenceId, Local);
	}
}

bool Protocol2::ReadProtocolId(UINT32& ProtocolId, const unsigned char*& Data, int& Len) const
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

	Data += SizeOfProtocolId;
	Len -= SizeOfProtocolId;

	return true;
}

bool Protocol2::ReadSequenceId(uint16_t& RemoteSequenceId, const unsigned char*& Data, int& Len) const
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

	Data += SizeOfSequenceId;
	Len -= SizeOfSequenceId;

	return true;
}

bool Protocol2::ReadAck(uint16_t& Ack, const unsigned char*& Data, int& Len) const
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

	Data += SizeOfAck;
	Len -= SizeOfAck;

	return true;
}

bool Protocol2::ReadAckBitfield(UINT32& AckBitfield, const unsigned char*& Data, int& Len, bool Skip) const
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

	Data += SizeOfAckBitfield;
	Len -= SizeOfAckBitfield;

	return true;
}

uint16_t Protocol2::GetLastSequenceId(SequenceIdType InSeqIdType) const
{
	if (InSeqIdType == Local)
	{
		return utils::sequence_modulo(LocalSequenceId - 1, std::numeric_limits<uint16_t>::max() - 1);
	}

	if (LastRemoteSequenceIdx == InvalidSequenceIdx)
	{
		return InvalidSequenceId;
	}

	return RemoteSequenceIdHistory[LastRemoteSequenceIdx];
}

bool Protocol2::WriteProtocolId(unsigned char*& Data, int& Len) const
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

bool Protocol2::WriteSequenceId(unsigned char*& Data, int& Len)
{
	constexpr std::size_t LocalSequenceIdLen = sizeof(uint16_t);
	if (Len < LocalSequenceIdLen)
	{
		printf(__FUNCTION__ ": buffer too small\n");
		return false;
	}

	uint16_t NetLocalSequenceId = htons(LocalSequenceId);
	memcpy(Data, &NetLocalSequenceId, LocalSequenceIdLen);
	Data += LocalSequenceIdLen;
	Len -= LocalSequenceIdLen;

	return true;
}

bool Protocol2::WriteAck(unsigned char*& Data, int& Len) const
{
	constexpr std::size_t AckLen = sizeof(uint16_t);
	constexpr std::size_t AckBitfieldLen = HistorySize / 8;
	if (Len < (AckLen + AckBitfieldLen))
	{
		printf(__FUNCTION__ ": buffer too small\n");
		return false;
	}

	// write remote sequence id after protocol id
	uint16_t LastRemoteSequenceId = GetLastSequenceId(Remote);

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

UINT32 Protocol2::ComputeAckBitfield(const uint16_t ReferenceSequenceId) const
{
	UINT32 AckBitfield = 0;
	for (int BitIdx = 0; BitIdx < ((sizeof(AckBitfield) * 8) - 1); ++BitIdx)
	{
		uint16_t RemoteSequenceIdToFind = utils::sequence_modulo(ReferenceSequenceId - (BitIdx + 1), std::numeric_limits<uint16_t>::max() - 1);
		constexpr int GoingBackward = -1;
		if (CheckPacket(RemoteSequenceIdToFind, Remote))
		{
			AckBitfield = 1 << BitIdx;
		}
	}

	return AckBitfield;
}

bool Protocol2::CheckPacket(const uint16_t InSequenceId, SequenceIdType InSeqType) const
{
	const uint16_t* InHistory = InSeqType == Local ? LocalSequenceIdHistory : RemoteSequenceIdHistory;

	const int Index = InSequenceId % HistorySize;
	return InHistory[Index] == InSequenceId;
}

void Protocol2::AckPacket(const uint16_t InAckedPacket)
{
	const int Index = InAckedPacket % HistorySize;
	LastAckedPackets[Index] = InAckedPacket;

	if (OnPacketAckedCallback)
	{
		OnPacketAckedCallback(InAckedPacket);
	}
}

bool Protocol2::ReadAndAck(const unsigned char*& Data, int& Len)
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
	for (int BitIdx = 0; BitIdx < ((sizeof(AckBitfield) * 8) - 1); ++BitIdx)
	{
		if (AckBitfield & (1 << BitIdx))
		{
			uint16_t SequenceId = Ack - BitIdx - 1;
			AckPacket(SequenceId);
		}
	}

	return true;
}

void Protocol2::CachePacket(uint16_t InSequenceId, SequenceIdType InSeqType)
{
	uint16_t* History = InSeqType == Local ? LocalSequenceIdHistory : RemoteSequenceIdHistory;

	const int Index = InSequenceId % HistorySize;
	History[Index] = InSequenceId;

	if (InSeqType == Local)
	{
		assert(InSequenceId == LocalSequenceId);
		++LocalSequenceId;
	}
	else
	{
		LastRemoteSequenceIdx = Index;
	}

}

const int Protocol2::Size() const
{
	return sizeof(unsigned int) + sizeof(uint16_t) + sizeof(uint16_t) + sizeof(UINT32);
}