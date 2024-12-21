#pragma once

#include <stdio.h>
#include <hermod/platform/Platform.h>
#include <hermod/serialization/Stream.h>

class Address;

class HERMOD_API UDPSocket
{
public:

	UDPSocket();
	UDPSocket(unsigned short port);
	virtual ~UDPSocket();

	bool SendTo(serialization::IStream& InStream, const Address& dest);
	bool SendTo(const unsigned char* data, int len, const Address& dest);
	int RecvFrom(Address& sender, unsigned char* data, int len);

	bool SetOption(long Option, DWORD Value);

	static bool Initialize();
	static void Shutdown();

private:
	bool Create(int af, int type, int protocol);

	bool Bind(unsigned short port = 0);

	SOCKET Handle;
	int Af;

};

