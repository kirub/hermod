#include "Mocks/MockBigPacket.h"
#include "Testables/ConnectionTestable.h"

#include <hermod/replication/NetObjectInterface.h>
#include <hermod/protocol/Connection.h>

#include <gtest/gtest.h>

void OnReceiveObject(const proto::INetObject& ReceivedPacket, const MockBigPacket& SentPacket)
{
    const MockBigPacket& BigPacketReceived = dynamic_cast<const MockBigPacket&>(ReceivedPacket);
    EXPECT_EQ(BigPacketReceived, SentPacket);
}

template <uint8_t FragmentCount>
void UnitTest_SendFragmented()
{
    uint8_t NumberOfFragments = FragmentCount;
    NetObjectManager::Get().Register<MockBigPacket>([NumberOfFragments]() { return new MockBigPacket(NumberOfFragments, MockBigPacket::None); });
    srand((unsigned int)time(NULL));
    bool ReceivedEqualSent = false;
    ConnectionTestable Connection;

    MockBigPacket Packet(FragmentCount);

    ASSERT_TRUE(Connection.Send((proto::INetObject&)Packet));
    MockBigPacket SentPacket = Packet;
    Packet.Reset();

    Connection.OnReceiveObject(std::bind(OnReceiveObject, std::placeholders::_1, SentPacket));

    Connection.Update(33.3f);
    NetObjectManager::Get().Unregister<MockBigPacket>();
}

template <uint8_t FragmentCount>
void UnitTest_SendIncompleteFragmented()
{
    uint8_t NumberOfFragments = FragmentCount;
    NetObjectManager::Get().Register<MockBigPacket>([NumberOfFragments]() { return new MockBigPacket(NumberOfFragments, MockBigPacket::None); });
    srand((unsigned int)time(NULL));
    bool ReceivedEqualSent = false;
    ConnectionTestable Connection;

    MockBigPacket Packet(FragmentCount, MockBigPacket::LastPacketHalf);

    ASSERT_TRUE(Connection.Send(Packet));
    MockBigPacket SentPacket = Packet;
    Packet.Reset();

    Connection.OnReceiveObject(std::bind(OnReceiveObject, std::placeholders::_1, SentPacket));

    Connection.Update(33.3f);
    NetObjectManager::Get().Unregister<MockBigPacket>();
}

TEST(Fragments, Send1FragmentedPacket)
{
    UnitTest_SendFragmented<1>();
}
TEST(Fragments, Send2FragmentedPacket)
{
    UnitTest_SendFragmented<2>();
}
TEST(Fragments, Send4FragmentedPacket)
{
    UnitTest_SendFragmented<4>();
}
TEST(Fragments, Send10FragmentedPacket)
{
    UnitTest_SendFragmented<10>();
}

TEST(Fragments, Send1IncompleteFragmentedPacket)
{
    UnitTest_SendIncompleteFragmented<1>();
}
TEST(Fragments, Send2IncompleteFragmentedPacket)
{
    UnitTest_SendIncompleteFragmented<2>();
}
TEST(Fragments, Send4IncompleteFragmentedPacket)
{
    UnitTest_SendIncompleteFragmented<4>();
}
TEST(Fragments, Send10IncompleteFragmentedPacket)
{
    UnitTest_SendIncompleteFragmented<10>();
}