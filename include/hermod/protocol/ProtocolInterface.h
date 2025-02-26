#pragma once

#include <hermod/platform/Platform.h>
#include <hermod/serialization/WriteStream.h>
#include <cstdint>
#include <functional>

namespace serialization
{
	class IStream;
}

class HERMOD_API IProtocol
{
public:
	static const uint16_t InvalidSequenceId;
	enum SequenceIdType
	{
		Local,
		Remote
	};

	using OnPacketAckedCallbackType = std::function<void(uint16_t)>;
	using OnPacketLostCallbackType = std::function<void(uint16_t)>;

	virtual bool WriteHeader(unsigned char* Data, int Len) = 0;
	virtual bool CheckHeader(const unsigned char*& Data, int& Len) = 0;
	virtual bool Serialize(serialization::IStream& InStream) = 0;
	virtual uint16_t GetLatestSequenceId(SequenceIdType InSeqIdType) const = 0;
	virtual const int Size() const = 0;
	virtual const int64_t GetRTT() const = 0;

	virtual bool HasPacketReliability() const { return false; }


	virtual uint16_t OnPacketSent(serialization::WriteStream InStream) { return InvalidSequenceId; }
	virtual uint16_t OnPacketSent(const uint16_t PacketSentSequenceId) { return InvalidSequenceId; };
	virtual uint16_t OnPacketSent(unsigned char* Buffer, int Len) = 0;
	virtual void OnPacketLost(const OnPacketLostCallbackType& Callback) {};
	virtual void OnPacketAcked(const OnPacketAckedCallbackType& Callback) {}
};
