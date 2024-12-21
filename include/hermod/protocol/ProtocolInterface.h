#pragma once

#include <hermod/platform/Platform.h>
#include <cstdint>

namespace serialization
{
	class IStream;
	class WriteStream;
}

class HERMOD_API IProtocol
{
public:
	using OnPacketAckedCallbackType = void(*)(uint16_t);

	virtual bool WriteHeader(unsigned char* Data, int Len) = 0;
	virtual bool CheckHeader(const unsigned char*& Data, int& Len) = 0;
	virtual bool Serialize(serialization::IStream& InStream) = 0;
	virtual const int Size() const = 0;


	virtual void OnPacketSent(serialization::WriteStream& InStream) {}
	virtual void OnPacketSent(const unsigned char* Buffer, int Len) = 0;
	virtual void OnPacketAcked(const OnPacketAckedCallbackType& Callback) {}
};