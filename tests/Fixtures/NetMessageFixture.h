#pragma once

#include <gtest/gtest.h>
#include <memory>

#include "Fixtures/NetFixture.h"
#include <hermod/socket/NetworkHandler.h>
#include <hermod/replication/NetObjectInterface.h>
#include "../Testables/ConnectionTestableLoopback.h"
#include "../Testables/ConnectionTestableWithMockSocket.h"
#include "../Vector2f.h"
#include "../Mocks/MockBigPacket.h"

class NetMessageFixture
    : public NetFixture
    , public testing::Test
{
protected:

    using NetHandlerType = NetworkHandler;

    NetMessageFixture()
        : NetMessageFixture(30000, 1)
    {
    }

    NetMessageFixture(unsigned short InPort, unsigned int InProtocolId)
        : ConnectionClientToServer(InPort)
        , CommonSocket(std::make_shared<UDPSocket>(InPort))
        , NetHandlerClient(&ConnectionClientToServer, std::make_shared<ProtocolTestable>(InProtocolId), CommonSocket)
        , NetHandlerServer(std::make_shared<ProtocolTestable>(InProtocolId), CommonSocket)
    {
        srand((unsigned int)time(NULL));
        NetObjectManager::Get().Register<Vector2f>();

        NetHandlerType::ConnectionHandlers::NewConnectionHandlerType NewConnectionFunc = [](const Address& InAddress) -> IConnection*
            {
                return new ConnectionTestableLoopback();
            };

        NetHandlerType::ConnectionHandlers::DeleteConnectionHandlerType DeleteConnectionFunc =
            [](IConnection* InConnection)
            {
                delete InConnection;
            };


        NetHandlerServer.SetConnectionHandlers({ NewConnectionFunc,DeleteConnectionFunc });
    }

    ~NetMessageFixture()
    {
        NetObjectManager::Get().Unregister<Vector2f>();
    }

    template < typename T>
    void OnReceiveObject(const proto::INetObject& ReceivedPacket, const T& SentPacket)
    {
        const T& PacketReceived = dynamic_cast<const T&>(ReceivedPacket);
        EXPECT_EQ(PacketReceived, SentPacket);
    }

    template < uint8_t NumMessages, std::derived_from<proto::INetObject> T>
    void SendMessages(const T& InitialMessage, std::function<T(const T&)> Transform)
    {
        T MessageTest = InitialMessage;
        for (uint8_t MessageIdx = 0; MessageIdx < NumMessages; ++MessageIdx)
        {
            ConnectionClientToServer.Send(MessageTest);
            MessageTest = Transform(MessageTest);
        }
    }

    template < uint8_t NumMessages, std::derived_from<proto::INetObject> T>
    void ReadMessages(const T& InitialMessage, std::function<T(const T&)> Transform)
    {
        T MessageTest = InitialMessage;
        for (uint8_t MessageIdx = 0; MessageIdx < NumMessages; ++MessageIdx)
        {
            proto::NetObjectPtr NetObject = NetHandlerServer.GetClientConnections()[0]->Receive();
            ASSERT_TRUE(NetObject);
            T ReceivedMessage = *std::static_pointer_cast<Vector2f>(NetObject);
            EXPECT_EQ(MessageTest, ReceivedMessage);
            MessageTest = Transform(ReceivedMessage);
        }
    }

    ConnectionTestableLoopback ConnectionClientToServer;
    std::shared_ptr<UDPSocket> CommonSocket;
    NetHandlerType NetHandlerServer;
    NetHandlerType NetHandlerClient;
};