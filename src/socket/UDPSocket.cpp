#include <hermod/socket/UDPSocket.h>

#include <hermod/socket/Address.h>

#include <stdio.h>

UDPSocket::UDPSocket()
	: UDPSocket(0)
{
}

UDPSocket::UDPSocket(unsigned short port)
	: Handle(0)
	, Af(AF_UNSPEC)
{
	if (!Create(AF_INET, SOCK_DGRAM, IPPROTO_UDP))
	{
		printf("failed to create socket!\n");
		return;
	}

	if (!Bind(port))
	{
		printf("failed to bind port!\n");
		return;
	}

	SetOption(FIONBIO, 1);
}

UDPSocket::~UDPSocket()
{
	if (Handle)
	{
		closesocket(Handle);
	}
}

bool UDPSocket::Create(int af, int type, int protocol)
{
	Af = af;

	Handle = socket(Af, type, protocol);
	if (Handle <= 0)
	{
		printf("failed to create socket\n");
		return false;
	}

	return true;
}

bool UDPSocket::SetOption(long Option, DWORD Value)
{
	if (ioctlsocket(Handle, Option, &Value) != 0)
	{
		printf("failed to set option %d \n", Option);
		return false;
	}

	return true;
}

bool UDPSocket::Bind(unsigned short port)
{
	sockaddr_in address;
	address.sin_family = Af;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(port);

	int Result = bind(Handle,
		(const sockaddr*)&address,
		sizeof(sockaddr_in));
	if (Result < 0)
	{
		printf("failed to bind socket: %d\n", Result);
		return false;
	}

	return true;
}


bool UDPSocket::SendTo(serialization::IStream& InStream, const Address& dest)
{
	const unsigned char* data = InStream.GetData();
	int len = InStream.GetDataSize();
	return SendTo(data, len, dest);
}

bool UDPSocket::SendTo(const unsigned char* data, int len, const Address& dest)
{
	const sockaddr_in to = dest.ToSockAddrIn();
	int sent_bytes = sendto(Handle, (const char*)data, len, 0, (const sockaddr*)&to, sizeof(sockaddr_in));
	if (sent_bytes != len)
	{
		printf("failed to send packet\n");
		return false;
	}

	return true;
}

int UDPSocket::RecvFrom(Address& sender, unsigned char* data, int len)
{
#if PLATFORM == PLATFORM_WINDOWS
	typedef int socklen_t;
#endif
	sockaddr_in from;
	socklen_t fromLength = sizeof(from);
	int bytes = recvfrom(Handle, (char*)data, len, 0, (sockaddr*)&from, &fromLength);
	if (bytes > 0)
	{
		sender = from;
	}
	return bytes;
}

bool UDPSocket::Initialize()
{
	WSADATA WsaData;
	return WSAStartup(MAKEWORD(2, 2), &WsaData) == NO_ERROR;
}

void UDPSocket::Shutdown()
{
	WSACleanup();
}

