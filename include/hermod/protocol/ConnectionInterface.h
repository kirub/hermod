#pragma once

#include <hermod/platform/Platform.h>
#include <hermod/utilities/Types.h>

#include <cstddef>

namespace proto
{
	class INetObject;
}

class HERMOD_API IConnection
{

public:

	using OnReceiveDataFunctor = void(*)(unsigned char*, const int);

	enum Error
	{
		None,
		DisconnectionError,
		InvalidRemoteAddress,
		InvalidHeader
	};

	virtual bool Send(proto::INetObject& Packet) = 0;
	virtual bool Send(unsigned char* Data, std::size_t Len) = 0;
	virtual const unsigned char* GetData() const = 0;

	virtual bool IsConnected() const = 0;
	virtual bool IsClient() const = 0;
	virtual bool IsServer() const = 0;

	virtual Error Update(TimeMs timeDelta) = 0;

	void OnReceiveData(OnReceiveDataFunctor InReceiveDataCallback);

protected:

	OnReceiveDataFunctor ReceiveDataCallback;
};