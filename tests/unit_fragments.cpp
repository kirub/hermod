#include "Mocks/MockBigPacket.h"
#include "Testables/ConnectionTestableWithMockSocket.h"
#include "Fixtures/NetMessageFixture.h"

#include <hermod/replication/NetObjectInterface.h>
#include <hermod/protocol/Connection.h>
#include <hermod/socket/NetworkHandler.h>

#include <gtest/gtest.h>



class NetFragmentationFixture
    : public NetMessageFixture
{
protected:

    template <uint8_t FragmentCount>
    void UnitTest_SendFragmented()
    {
        uint8_t NumberOfFragments = FragmentCount;
        NetObjectManager::Get().Register<MockBigPacket>([NumberOfFragments]() { return new MockBigPacket(NumberOfFragments, MockBigPacket::None); });

        MockBigPacket Packet(FragmentCount);
        ASSERT_TRUE(ConnectionClientToServer.Send(Packet));
        NetHandlerClient.SendAllMessages(ConnectionClientToServer);

        // ---------------------------
        NetHandlerServer.OnStartFrame();
        ASSERT_GT(NetHandlerServer.GetClientConnections().size(), 0);

        proto::NetObjectPtr NetObject = NetHandlerServer.GetClientConnections()[0]->Receive();
        ASSERT_TRUE(NetObject);
        EXPECT_EQ(Packet, *std::static_pointer_cast<MockBigPacket>(NetObject)); 

        NetObjectManager::Get().Unregister<MockBigPacket>();
    }

    template <uint8_t FragmentCount>
    void UnitTest_SendIncompleteFragmented()
    {
        uint8_t NumberOfFragments = FragmentCount;
        NetObjectManager::Get().Register<MockBigPacket>([NumberOfFragments]() { return new MockBigPacket(NumberOfFragments, MockBigPacket::None); });

        MockBigPacket Packet(FragmentCount, MockBigPacket::LastPacketHalf);
        ASSERT_TRUE(ConnectionClientToServer.Send(Packet));
        NetHandlerClient.SendAllMessages(ConnectionClientToServer);

        // ---------------------------

        NetHandlerServer.OnStartFrame();
        ASSERT_GT(NetHandlerServer.GetClientConnections().size(), 0);

        proto::NetObjectPtr NetObject = NetHandlerServer.GetClientConnections()[0]->Receive();
        ASSERT_TRUE(NetObject);
        EXPECT_EQ(Packet, *std::static_pointer_cast<MockBigPacket>(NetObject));

        NetObjectManager::Get().Unregister<MockBigPacket>();
    }
};

TEST_F(NetFragmentationFixture, Send1FragmentedPacket)
{
    UnitTest_SendFragmented<1>();
}
TEST_F(NetFragmentationFixture, Send2FragmentedPacket)
{
    UnitTest_SendFragmented<2>();
}
TEST_F(NetFragmentationFixture, Send4FragmentedPacket)
{
    UnitTest_SendFragmented<4>();
}
TEST_F(NetFragmentationFixture, Send10FragmentedPacket)
{
    UnitTest_SendFragmented<10>();
}

TEST_F(NetFragmentationFixture, Send1IncompleteFragmentedPacket)
{
    UnitTest_SendIncompleteFragmented<1>();
}
TEST_F(NetFragmentationFixture, Send2IncompleteFragmentedPacket)
{
    UnitTest_SendIncompleteFragmented<2>();
}
TEST_F(NetFragmentationFixture, Send4IncompleteFragmentedPacket)
{
    UnitTest_SendIncompleteFragmented<4>();
}
TEST_F(NetFragmentationFixture, Send10IncompleteFragmentedPacket)
{
    UnitTest_SendIncompleteFragmented<10>();
}