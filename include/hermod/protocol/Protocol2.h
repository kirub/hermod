#pragma once

#include <hermod/platform/Platform.h>

#include <stdio.h>
#include <queue>
#include <functional>
#include "Protocol.h"



class Protocol2
	: public IProtocol
{
public:

	using OnPacketAckedCallbackType = std::function<void(uint16_t)>;

	Protocol2(unsigned int InId);

	bool WriteHeader(unsigned char* Data, int Len);
	bool CheckHeader(const unsigned char*& Data, int& Len);

	virtual void OnPacketSent(const unsigned char* Buffer, int Len);
	virtual void OnPacketAcked(const OnPacketAckedCallbackType& Callback);

	virtual const int Size() const;


private:

	enum SequenceIdType
	{
		Local,
		Remote
	};

	static const uint16_t HistorySize = 1024;
	static const uint16_t InvalidSequenceIdx = std::numeric_limits<uint16_t>::max();
	static const uint16_t InvalidSequenceId = std::numeric_limits<uint16_t>::max();

	bool WriteProtocolId(unsigned char*& Data, int& Len) const;
	bool WriteSequenceId(unsigned char*& Data, int& Len);
	bool WriteAck(unsigned char*& Data, int& Len) const;

	bool ReadSequenceId(uint16_t& OutSequenceId, const unsigned char*& Data, int& Len) const;
	bool ReadAck(uint16_t& OutAck, const unsigned char*& Data, int& Len) const;
	bool ReadProtocolId(UINT32& OutProtocolId, const unsigned char*& Data, int& Len) const;
	bool ReadAckBitfield(UINT32& OutAckBitfield, const unsigned char*& Data, int& Len, bool Skip = false) const;

	uint16_t GetLastSequenceId(SequenceIdType InSeqIdType) const;

	bool ReadAndAck(const unsigned char*& Data, int& Len);
	bool CheckPacket(const uint16_t InPacketId, SequenceIdType InSeqType) const;
	UINT32 ComputeAckBitfield(const uint16_t LastRemoteSequenceId) const;

	void AckPacket(const uint16_t InAckedPacket);
	void CachePacket(uint16_t NewSequenceId, SequenceIdType InSeqType);

	OnPacketAckedCallbackType OnPacketAckedCallback;

	unsigned int Id;
	uint16_t LocalSequenceId;
	uint16_t LastRemoteSequenceIdx;
	uint16_t LocalSequenceIdHistory[HistorySize];
	uint16_t RemoteSequenceIdHistory[HistorySize];
	uint16_t LastAckedPackets[HistorySize];
};