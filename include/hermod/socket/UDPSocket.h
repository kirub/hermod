#pragma once

#include <stdio.h>
#include <hermod/platform/Platform.h>
#include <hermod/socket/SocketInterface.h>

class HERMOD_API UDPSocket
	: public ISocket
{
public:
	using ISocket::Send;

	UDPSocket();
	UDPSocket(unsigned short port);
	virtual ~UDPSocket();

	virtual bool Send(const unsigned char* data, int len, const Address& dest) override;
	virtual int Receive(Address& sender, unsigned char* data, int len) override;

	bool SetOption(long Option, DWORD Value);

private:
	bool Create(int af, int type, int protocol);

	bool Bind(unsigned short port = 0);

	int Af;

};

