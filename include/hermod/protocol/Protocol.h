#pragma once

#include "ProtocolInterface.h"
#include <hermod/platform/Platform.h>
#include <memory>
#include <chrono>

namespace serialization
{
	class IStream;
	class WriteStream;
}

class Protocol
	: public IProtocol
{
public:

	HERMOD_API Protocol(unsigned int InId);

	HERMOD_API virtual bool WriteHeader(unsigned char* Data, int Len) override;
	HERMOD_API virtual bool CheckHeader(const unsigned char*& Data, int& Len) override;
	HERMOD_API virtual bool Serialize(serialization::IStream& InStream) override;

	HERMOD_API virtual uint16_t OnPacketSent(serialization::WriteStream InStream) override;
	HERMOD_API virtual uint16_t OnPacketSent(const uint16_t PacketSentSequenceId) override;
	HERMOD_API virtual uint16_t OnPacketSent(unsigned char* Buffer, int Len) override;
	HERMOD_API virtual void OnPacketLost(const OnPacketLostCallbackType& Callback) override;
	HERMOD_API virtual void OnPacketAcked(const OnPacketAckedCallbackType& Callback) override;

	HERMOD_API virtual uint16_t GetLatestSequenceId(SequenceIdType InSeqIdType) const override;
	HERMOD_API virtual const int Size() const override;
	HERMOD_API virtual const int64_t GetRTT() const override;

	HERMOD_API virtual bool HasPacketReliability() const override { return true; }
	
protected:


	static const UINT8 HistorySize = 33;
	static const UINT8 InvalidSequenceIdx = 255;
	static const int64_t MaxRTT = 1000;
	static const uint8_t RTTIncFactor = 10;


	bool WriteProtocolId(unsigned char*& Data, int& Len) const;
	bool WriteSequenceId(unsigned char*& Data, int& Len);
	bool WriteAck(unsigned char*& Data, int& Len) const;

	bool ReadSequenceId(uint16_t& OutSequenceId, const unsigned char*& Data, int& Len) const;
	bool ReadAck(uint16_t& OutAck, const unsigned char*& Data, int& Len) const;
	bool ReadProtocolId(UINT32& OutProtocolId, const unsigned char*& Data, int& Len) const;
	bool ReadAckBitfield(UINT32& OutAckBitfield, const unsigned char*& Data, int& Len, bool Skip = false) const;


	UINT8 GetLatestSequenceIdx( SequenceIdType InSeqIdType ) const;
	UINT8 GetNextSequenceIdx(SequenceIdType InSeqIdType) const;

	uint16_t GetNextSequenceId(SequenceIdType InSeqIdType) const;

	void ReportRTT(const int64_t InRTT);
	bool ReadAndAck(const unsigned char*& Data, int& Len);
	UINT8 FindPacket(const uint16_t InPacketId, SequenceIdType InSeqType, const UINT8 StartIdx = 0, const int Increment = 1) const;
	bool CheckPacket(const uint16_t InPacketId, SequenceIdType InSeqType) const;
	UINT32 ComputeAckBitfield(const uint16_t LastRemoteSequenceId) const;
	UINT32 ComputeAckBitfield2(const uint16_t ReferenceSequenceId) const;
	HERMOD_API void TriggerPacketLost(uint16_t PacketSequenceId) const;

	HERMOD_API virtual void AckPacket(const uint16_t InAckedPacket);
	HERMOD_API virtual void CachePacket(uint16_t NewSequenceId, SequenceIdType InSeqType);

private:

	struct PacketData
	{
		uint16_t SequenceId = InvalidSequenceId;
		int64_t SendTime = -1;

		const int64_t RTT()
		{
			return (SendTime > -1) ? utils::Time::NowMs() - SendTime : 0;
		}

		void Reset()
		{
			SequenceId = InvalidSequenceId;
			SendTime = -1;
		}

		PacketData& operator=(uint16_t InSequenceId)
		{
			SequenceId = InSequenceId;
			SendTime = InSequenceId == InvalidSequenceId ? -1 : utils::Time::NowMs();
			return *this;
		}
	};

	OnPacketLostCallbackType OnPacketLostCallback;
	OnPacketAckedCallbackType OnPacketAckedCallback;

	int64_t CurrentRTT;
	unsigned int Id;
	uint16_t LocalSequenceIdHistory[HistorySize];
	uint16_t RemoteSequenceIdHistory[HistorySize];
	PacketData NotAckedPackets[HistorySize];
	UINT8 NextLocalSequenceIdx;
	UINT8 NextRemoteSequenceIdx;
};

