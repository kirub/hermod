#pragma once

#include <hermod/platform/Platform.h>
#include <hermod/serialization/WriteStream.h>
#include <cstdint>
#include <functional>

namespace serialization
{
	class IStream;
}

template < typename ReturnType, typename... TArgs>
class Callable
{
public:
	using CallbackType = ReturnType(*)(TArgs...);

	Callable(CallbackType InCallback)
		: Callback(InCallback)
	{ }

	ReturnType operator()(TArgs... Args) const
	{
		Call(Args...);
	}

	virtual ReturnType Call(TArgs... Args) const
	{
		return Callback(Args...);
	}

	virtual bool IsValid() const
	{
		return Callback != nullptr;
	}

	operator bool() const
	{
		return IsValid();
	}

protected:
	CallbackType Callback;
};
/*
template < typename InOwnerType, typename InReturnType, typename... TArgs>
class CallableWithOwner
	: public Callable<InReturnType, TArgs...>
{
public:
	using OwnerType = InOwnerType;
	using ReturnType = InReturnType;
	using CallbackTypeWithOwner = ReturnType(OwnerType::*)(TArgs...);

	CallableWithOwner(OwnerType& InOwner, CallbackTypeWithOwner InCallback)
		: Callable<ReturnType, TArgs...>(nullptr)
		, Owner(InOwner)
		, CallbackWithOwner(InCallback)
	{ }

	virtual ReturnType Call(TArgs... Args) const
	{
		return (Owner.*CallbackWithOwner)(Args...);
	}

	virtual bool IsValid() const
	{
		return CallbackWithOwner != nullptr;
	}
private:

	OwnerType& Owner;
	CallbackTypeWithOwner CallbackWithOwner;
};
*/
class HERMOD_API IProtocol
{
public:
	static const uint16_t InvalidSequenceId;

	using OnPacketAckedCallbackType = std::function<void(uint16_t)>;
	using OnPacketLostCallbackType = std::function<void(uint16_t)>;

	virtual bool WriteHeader(unsigned char* Data, int Len) = 0;
	virtual bool CheckHeader(const unsigned char*& Data, int& Len) = 0;
	virtual bool Serialize(serialization::IStream& InStream) = 0;
	virtual const int Size() const = 0;
	virtual const int64_t GetRTT() const = 0;


	virtual uint16_t OnPacketSent(serialization::WriteStream InStream) { return InvalidSequenceId; }
	virtual uint16_t OnPacketSent(const uint16_t PacketSentSequenceId, serialization::WriteStream InStream) { return InvalidSequenceId; };
	virtual uint16_t OnPacketSent(unsigned char* Buffer, int Len) = 0;
	virtual void OnPacketLost(const OnPacketLostCallbackType& Callback) {};
	virtual void OnPacketAcked(const OnPacketAckedCallbackType& Callback) {}
};