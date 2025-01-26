#include "MockSocket.h"

MockSocket::MockSocket(unsigned short port)
{
}

std::size_t MockSocket::DataAvailable()
{
    return Packets.size();
}

bool MockSocket::Send(const unsigned char* data, int len, const Address& dest)
{
    Packets.push(MockUDPPacket((uint8_t*)data, len));
    SendAddr = dest;
    return true;
}
int MockSocket::Receive(Address& sender, unsigned char* data, int len)
{
    int ReceiveBytesCount = 0;
    if (len > 0 && DataAvailable() > 0)
    {
        sender = SendAddr;
        MockUDPPacket& Packet = Packets.front();
        ReceiveBytesCount = Packet.Read(data, len);
        Packets.pop();
    }
    return ReceiveBytesCount;
}