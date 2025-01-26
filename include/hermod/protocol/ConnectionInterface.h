#pragma once

#include <hermod/platform/Platform.h>
#include <hermod/utilities/Types.h>
#include <hermod/replication/NetObjectInterface.h>

#include <cstddef>

namespace serialization
{
	class WriteStream;
}

class HERMOD_API IConnection
{

public:

	using OnReceiveDataFunctor = void(*)(unsigned char*, const int);
	using OnReceiveObjectFunctor = std::function<void(const proto::INetObject& Object)>;

	enum Error
	{
		None,
		DisconnectionError,
		InvalidRemoteAddress,
		InvalidHeader
	};

	IConnection()
		: ReceiveDataCallback(nullptr)
		, ReceiveObjectCallback(nullptr)
	{

	}

	virtual bool Send(serialization::WriteStream& Packet, EReliability InReliability = Unreliable, bool IsResend = false) = 0;
	virtual bool Send(proto::INetObject& Packet, EReliability InReliability = Unreliable) = 0;
	virtual bool Send(unsigned char* Data, std::size_t Len) = 0;
	virtual const unsigned char* GetData() = 0;

	virtual bool IsConnected() const = 0;
	virtual bool IsClient() const = 0;
	virtual bool IsServer() const = 0;

	virtual Error Update(TimeMs timeDelta) = 0;

	void OnReceiveData(OnReceiveDataFunctor InReceiveDataCallback)
	{
		ReceiveDataCallback = InReceiveDataCallback;
	}
	void OnReceiveObject(OnReceiveObjectFunctor InReceiveObjectCallback)
	{
		ReceiveObjectCallback = InReceiveObjectCallback;
	}

protected:

	OnReceiveDataFunctor ReceiveDataCallback;
	OnReceiveObjectFunctor ReceiveObjectCallback;
};