#pragma once

#include <stdio.h>
#include <hermod/platform/Platform.h>
#include <hermod/serialization/Stream.h>

class Address;

class HERMOD_API ISocket
{
public:

	virtual bool Send(const unsigned char* data, int len, const Address& dest) = 0;
	virtual int Receive(Address& sender, unsigned char* data, int len) = 0;


	bool Send(serialization::IStream& InStream, const Address& dest);
	int Receive(Address& sender, serialization::IStream& Stream);


    static bool Initialize()
    {
        WSADATA WsaData;
        return WSAStartup(MAKEWORD(2, 2), &WsaData) == NO_ERROR;
    }

    static void Shutdown()
    {
        WSACleanup();
    }

	SOCKET Handle;
};