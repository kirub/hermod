#include "Mocks/MockBigPacket.h"

#include <gtest/gtest.h>
#include "Testables/ConnectionTestableWithMockSocket.h"
#include "Vector2f.h"

class TestFixtureReliability
    : public testing::Test
{
protected:

    TestFixtureReliability()
        : Connection(10000)
    {
        srand((unsigned int)time(NULL));
        NetObjectManager::Get().Register<MockBigPacket>([]() { return new MockBigPacket(0, MockBigPacket::None); });
    }
    ~TestFixtureReliability()
    {
        NetObjectManager::Get().Unregister<MockBigPacket>();
    }

    template < typename T>
    void OnReceiveObject(const proto::INetObject& ReceivedPacket, const T& SentPacket)
    {
        const T& PacketReceived = dynamic_cast<const T&>(ReceivedPacket);
        EXPECT_EQ(PacketReceived, SentPacket);
    }

    template < typename T>
    void SendAndTest(T& NetObject)
    {
        ASSERT_TRUE(Connection.Send(NetObject));
    }

    ConnectionTestableWithMockSocket Connection;
};
/*
TEST_F(TestFixtureReliability, SendReliablePacket_RetrySendAfterPacketLost)
{
    Vector2f Packet(123.456f,789.123f);

    Connection.SimulateNextPacketLost();
    SendAndTestEquality(Packet, Reliable);
    uint16_t InitialPacketId = Connection.ProtoTest->GetLastSentPacketId();
    Connection.ProtoTest->TriggerPacketLost(InitialPacketId);
    uint16_t ResentPacketId = Connection.ProtoTest->GetLastSentPacketId();
    Connection.Update(33.3f);
    uint16_t ReceivedPacketId = Connection.ProtoTest->GetLastReceivedtPacketId();

    EXPECT_EQ(InitialPacketId + 1, ResentPacketId);
    EXPECT_EQ(ResentPacketId, ReceivedPacketId);


}*/