#pragma once

#include <hermod/platform/Platform.h>
#include <hermod/utilities/Types.h>
#include <hermod/replication/NetObjectInterface.h>

#include <cstddef>

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

	virtual bool Send(proto::INetObject& Packet) = 0;
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
	void SetId(int InConnectionId)
	{
		ConnectionId = InConnectionId;
	}
	int GetId() const
	{
		return ConnectionId;
	}

protected:

	OnReceiveDataFunctor ReceiveDataCallback;
	OnReceiveObjectFunctor ReceiveObjectCallback;
	int ConnectionId;
};