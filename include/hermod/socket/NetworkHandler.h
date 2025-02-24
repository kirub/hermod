#pragma once

#include <vector>
#include <memory>
#include <hermod/utilities/Callable.h>
#include <hermod/serialization/ReadStream.h>
#include <hermod/serialization/WriteStream.h>
#include <hermod/socket/Address.h>

class IProtocol;
class ISocket;
class IConnection;


class NetworkHandler
{
public:
    using ProtocolPtr = std::shared_ptr<IProtocol>;
    using SocketPtr = std::shared_ptr<ISocket>;

    class ConnectionHandlers
    {
    public:
        using NewConnectionHandlerType = Callable<IConnection*(const Address&)>;
        using DeleteConnectionHandlerType = Callable<void(IConnection*)>;

        HERMOD_API ConnectionHandlers();

        HERMOD_API ConnectionHandlers(NewConnectionHandlerType InOnNewConnection, DeleteConnectionHandlerType InOnDeleteConnection);

        IConnection* OnNewConnection(Address InAddress);
        void OnDeleteConnection(IConnection* InConnection);

    private:
        NewConnectionHandlerType OnNewConnectionFunc;
        DeleteConnectionHandlerType OnDeleteConnectionFunc;
    };

    HERMOD_API NetworkHandler(ProtocolPtr InProtocol, SocketPtr InSocket);

    HERMOD_API NetworkHandler(IConnection* InServerConnection, ProtocolPtr InProtocol, SocketPtr InSocket);

    HERMOD_API ~NetworkHandler();

    HERMOD_API const std::vector<IConnection*>& GetClientConnections() const;
    HERMOD_API const IConnection* GetServerConnection() const;

    HERMOD_API void SetConnectionHandlers(ConnectionHandlers InNewConnectionHandlers);
    HERMOD_API void OnStartFrame();
    HERMOD_API void OnEndFrame();
    HERMOD_API void SendAllMessages(IConnection& InConnection);
private:

    void AddConnection(IConnection* InConnection);
    int RemoveConnection(IConnection* InConnection);

    bool IsServer() const;

    IConnection* FindConnection(const Address& InAddress);

    void Read();

    int32_t Write();

    int32_t WriteOnConnection(IConnection* InConnection);

    static const int MessageHistorySize = 256;

    serialization::WriteStream          WriteStream;
    serialization::ReadStream           ReadStream;

    FD_SET ReadWriteSet;

    IConnection*                        ServerConnection;
    ProtocolPtr                         NetProtocol;
    SocketPtr                           Socket;
    std::vector<IConnection*>           Connections;

    ConnectionHandlers                  ConnectionHandlersFunc;

    // If Packet Reliability
    static const int PacketSentHistorySize = 64;
    serialization::WriteStream PacketsSent[PacketSentHistorySize];
};