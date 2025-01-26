#pragma once

#include "MockUDPPacket.h"

#include <gtest/gtest.h>
#include <hermod/socket/SocketInterface.h>
#include <hermod/replication/NetObjectInterface.h>
#include <hermod/protocol/Connection.h>
#include <queue>

class MockSocket
    : public ISocket
{
    Address SendAddr;
    std::queue<MockUDPPacket> Packets;

public:

    MockSocket(unsigned short port);

    std::size_t DataAvailable();

    virtual bool Send(const unsigned char* data, int len, const Address& dest) override;
    virtual int Receive(Address& sender, unsigned char* data, int len) override;
};