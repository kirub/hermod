#pragma once

#include "ProtocolInterface.h"
#include <hermod/platform/Platform.h>
#include <memory>

namespace serialization
{
	class IStream;
	class WriteStream;
}

class HERMOD_API Protocol
	: public IProtocol
{
public:

	Protocol(unsigned int InId);

	virtual bool WriteHeader(unsigned char* Data, int Len) override;
	virtual bool CheckHeader(const unsigned char*& Data, int& Len) override;
	virtual bool Serialize(serialization::IStream& InStream) override;

	virtual uint16_t OnPacketSent(serialization::WriteStream InStream) override;
	virtual uint16_t OnPacketSent(const uint16_t PacketSentSequenceId, serialization::WriteStream InStream) override;
	virtual uint16_t OnPacketSent(unsigned char* Buffer, int Len) override;
	virtual void OnPacketLost(const OnPacketLostCallbackType& Callback) override;
	virtual void OnPacketAcked(const OnPacketAckedCallbackType& Callback) override;

	virtual const int Size() const;

	
protected:

	enum SequenceIdType
	{
		Local,
		Remote
	};

	static const UINT8 HistorySize = 33;
	static const UINT8 InvalidSequenceIdx = 255;


	bool WriteProtocolId(unsigned char*& Data, int& Len) const;
	bool WriteSequenceId(unsigned char*& Data, int& Len);
	bool WriteAck(unsigned char*& Data, int& Len) const;

	bool ReadSequenceId(uint16_t& OutSequenceId, const unsigned char*& Data, int& Len) const;
	bool ReadAck(uint16_t& OutAck, const unsigned char*& Data, int& Len) const;
	bool ReadProtocolId(UINT32& OutProtocolId, const unsigned char*& Data, int& Len) const;
	bool ReadAckBitfield(UINT32& OutAckBitfield, const unsigned char*& Data, int& Len, bool Skip = false) const;


	UINT8 GetLatestSequenceIdx( SequenceIdType InSeqIdType ) const;
	UINT8 GetNextSequenceIdx(SequenceIdType InSeqIdType) const;

	uint16_t GetLatestSequenceId(SequenceIdType InSeqIdType) const;
	uint16_t GetNextSequenceId(SequenceIdType InSeqIdType) const;

	bool ReadAndAck(const unsigned char*& Data, int& Len);
	UINT8 FindPacket(const uint16_t InPacketId, SequenceIdType InSeqType, const UINT8 StartIdx = 0, const int Increment = 1) const;
	bool CheckPacket(const uint16_t InPacketId, SequenceIdType InSeqType) const;
	UINT32 ComputeAckBitfield(const uint16_t LastRemoteSequenceId) const;
	UINT32 ComputeAckBitfield2(const uint16_t ReferenceSequenceId) const;
	void TriggerPacketLost(uint16_t PacketSequenceId) const;

	virtual void AckPacket(const uint16_t InAckedPacket);
	virtual void CachePacket(uint16_t NewSequenceId, SequenceIdType InSeqType);

	OnPacketLostCallbackType OnPacketLostCallback;
	OnPacketAckedCallbackType OnPacketAckedCallback;

	unsigned int Id;
	uint16_t LocalSequenceIdHistory[HistorySize];
	uint16_t RemoteSequenceIdHistory[HistorySize];
	uint16_t NotAckedPackets[HistorySize];
	UINT8 NextLocalSequenceIdx;
	UINT8 NextRemoteSequenceIdx;
};

