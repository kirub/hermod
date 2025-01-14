#include <hermod/socket/SocketInterface.h>
#include <hermod/serialization/Stream.h>

bool ISocket::Send(serialization::IStream& InStream, const Address& dest)
{
	const unsigned char* data = InStream.GetData();
	int len = InStream.GetDataSize();
	return Send(data, len, dest);
}

int ISocket::Receive(Address& sender, serialization::IStream& Stream)
{
	int BytesRead = Receive(sender, (unsigned char*)Stream.GetData(), Stream.GetBytesRemaining());
	Stream.AdjustSize(BytesRead);
	return BytesRead;
}

bool ISocket::Initialize()
{
	WSADATA WsaData;
	return WSAStartup(MAKEWORD(2, 2), &WsaData) == NO_ERROR;
}

void ISocket::Shutdown()
{
	WSACleanup();
}