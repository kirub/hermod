#pragma once

#include <hermod/utilities/Types.h>

struct MockUDPPacket
{
    static const int bufferSize = MaxMTUSize;
    int PacketSize;
    uint8_t buffer[bufferSize];

    MockUDPPacket(uint8_t* InBuffer, int InSize);

    int Read(uint8_t* data, int MaxSize);
};