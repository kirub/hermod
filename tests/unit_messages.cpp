
#include <gtest/gtest.h>
#include <memory>

#include <hermod/replication/NetObjectInterface.h>
#include <hermod/socket/NetworkHandler.h>
#include "Vector2f.h"
#include "Testables/ConnectionTestableWithMockSocket.h"
#include "Fixtures/NetMessageFixture.h"

TEST_F(NetMessageFixture, HandleNewConnection)
{
    // Client send to server
    ASSERT_NE(NetHandlerClient.GetServerConnection(), nullptr);
    Vector2f Test(123.456f, 456.789f);
    ConnectionClientToServer.Send(Test);
    NetHandlerClient.OnEndFrame();

    // ---------------------------
    NetHandlerServer.OnStartFrame();
    ASSERT_GT(NetHandlerServer.GetClientConnections().size(), 0);
}

TEST_F(NetMessageFixture, Send1Message)
{
    // Client send to server
    ASSERT_NE(NetHandlerClient.GetServerConnection(), nullptr);
    Vector2f Test(123.456f, 456.789f);
    ConnectionClientToServer.Send(Test);
    NetHandlerClient.OnEndFrame();

    // ---------------------------
    NetHandlerServer.OnStartFrame();
    ASSERT_GT(NetHandlerServer.GetClientConnections().size(), 0);

    proto::NetObjectPtr NetObject = NetHandlerServer.GetClientConnections()[0]->Receive();
    ASSERT_TRUE(NetObject);
    EXPECT_EQ(Test, *std::static_pointer_cast<Vector2f>(NetObject));
}


// TEST_P(FooTest, DoesBar) {
//   // Can use GetParam() method here.
//   Foo foo;
//   ASSERT_TRUE(foo.DoesBar(GetParam()));
// }

TEST_F(NetMessageFixture, Send2Messages)
{
    // Client send to server
    ASSERT_NE(NetHandlerClient.GetServerConnection(), nullptr);
    Vector2f MessageTest(1.2f, 2.3f);
    std::function<Vector2f(const Vector2f&)> TransformFunc = [](const Vector2f& PreviousMessage) -> Vector2f
        {
            return { PreviousMessage.GetX() * 2.0f, PreviousMessage.GetY() * 2.0f };
        };
    SendMessages<2>(MessageTest, TransformFunc);
    NetHandlerClient.OnEndFrame();

    // ---------------------------
    NetHandlerServer.OnStartFrame();
    ASSERT_GT(NetHandlerServer.GetClientConnections().size(), 0);

    ReadMessages<2>(MessageTest, TransformFunc);
}

