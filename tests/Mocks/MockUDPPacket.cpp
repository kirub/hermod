#include "MockUDPPacket.h"
#include <utility>
#include <cassert>

MockUDPPacket::MockUDPPacket(uint8_t* InBuffer, int InSize)
    : PacketSize(InSize)
{
    assert(bufferSize >= InSize);
    memcpy(buffer, InBuffer, std::min(InSize, bufferSize));
}

int MockUDPPacket::Read(uint8_t* data, int MaxSize)
{
    assert(MaxSize > -PacketSize);
    int ReceiveBytesCount = std::min(PacketSize, MaxSize);
    memcpy(data, buffer, ReceiveBytesCount);
    return ReceiveBytesCount;
}