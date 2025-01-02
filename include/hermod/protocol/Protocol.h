#pragma once

#include "ProtocolInterface.h"
#include <hermod/platform/Platform.h>

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

	virtual bool WriteHeader(unsigned char* Data, int Len);
	virtual bool CheckHeader(const unsigned char*& Data, int& Len);
	virtual bool Serialize(serialization::IStream& InStream);

	virtual void OnPacketSent(serialization::WriteStream& InStream);
	virtual void OnPacketSent(const unsigned char* Buffer, int Len);
	virtual void OnPacketAcked(const IProtocol::OnPacketAckedCallbackType& Callback);

	virtual const int Size() const;

	
private:

	enum SequenceIdType
	{
		Local,
		Remote
	};

	static const UINT8 HistorySize = 33;
	static const UINT8 InvalidSequenceIdx = 255;
	static const uint16_t InvalidSequenceId;

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

	void AckPacket(const uint16_t InAckedPacket);
	void CachePacket(uint16_t NewSequenceId, SequenceIdType InSeqType);
	void CachePacket2(uint16_t NewSequenceId, SequenceIdType InSeqType);

	OnPacketAckedCallbackType OnPacketAckedCallback;

	unsigned int Id;
	uint16_t LocalSequenceIdHistory[HistorySize];
	uint16_t RemoteSequenceIdHistory[HistorySize];
	uint16_t LastAckedPackets[HistorySize];
	UINT8 NextLocalSequenceIdx;
	UINT8 NextRemoteSequenceIdx;
};

