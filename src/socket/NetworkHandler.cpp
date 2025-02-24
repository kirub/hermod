#include <hermod/socket/NetworkHandler.h>
#include <hermod/platform/Platform.h>
#include <hermod/utilities/Types.h>
#include <hermod/protocol/ProtocolInterface.h>
#include <hermod/socket/SocketInterface.h>
#include <hermod/protocol/ConnectionInterface.h>
#include <hermod/protocol/NetObjectQueue.h>

NetworkHandler::ConnectionHandlers::ConnectionHandlers()
    : ConnectionHandlers(NewConnectionHandlerType(), DeleteConnectionHandlerType())
{ }

NetworkHandler::ConnectionHandlers::ConnectionHandlers(NewConnectionHandlerType InOnNewConnection, DeleteConnectionHandlerType InOnDeleteConnection)
    : OnNewConnectionFunc(InOnNewConnection)
    , OnDeleteConnectionFunc(InOnDeleteConnection)
{
}

IConnection* NetworkHandler::ConnectionHandlers::OnNewConnection(Address InAddress)
{
    if (!OnNewConnectionFunc)
    {
        printf("No new connection handler provided!");
        return nullptr;
    }

    return OnNewConnectionFunc(std::move(InAddress));
}
void NetworkHandler::ConnectionHandlers::OnDeleteConnection(IConnection* InConnection)
{
    if (OnDeleteConnectionFunc && InConnection)
    {
        OnDeleteConnectionFunc(InConnection);
    }
}

NetworkHandler::NetworkHandler(ProtocolPtr InProtocol, SocketPtr InSocket)
    : NetworkHandler(nullptr, InProtocol, InSocket)
{
}

NetworkHandler::NetworkHandler(IConnection* InServerConnection, ProtocolPtr InProtocol, SocketPtr InSocket)
        : ServerConnection(InServerConnection)
        , NetProtocol(InProtocol)
        , Socket(InSocket)
        , WriteStream(MaxMTUSize)
        , ReadStream(MaxMTUSize)
    {
        FD_ZERO(&ReadWriteSet);
        FD_SET(Socket->Handle, &ReadWriteSet);
    }

NetworkHandler::~NetworkHandler()
    {
        for (IConnection* Connection : Connections)
        {
            ConnectionHandlersFunc.OnDeleteConnection(Connection);            
        }
        Connections.clear();
    }

    void NetworkHandler::AddConnection(IConnection* InConnection)
    {
        Connections.push_back(InConnection);
    }
    int NetworkHandler::RemoveConnection(IConnection* InConnection)
    {        
        std::vector<IConnection*>::iterator Found = std::find(Connections.begin(), Connections.end(), InConnection);
        int Index = (int)( Found - Connections.begin());
        Connections.erase(Found);
        return Index;
    }

    const std::vector<IConnection*>& NetworkHandler::GetClientConnections() const
    {
        return Connections;
    }

    const IConnection* NetworkHandler::GetServerConnection() const
    {
        return ServerConnection;
    }

    void NetworkHandler::SetConnectionHandlers(ConnectionHandlers InNewConnectionHandlers)
    {
        ConnectionHandlersFunc = InNewConnectionHandlers;
    }

    void NetworkHandler::OnStartFrame()
    {
        Read();
    }

    void NetworkHandler::OnEndFrame()
    {
        Write();
    }
    void NetworkHandler::SendAllMessages(IConnection& InConnection)
    {
        proto::NetObjectQueue256& NetObjectQueue = InConnection.GetNetObjectQueue(IConnection::SendQueue);
        int32_t MessagesCount = NetObjectQueue.Size();
        if (MessagesCount > 0)
        {
            int32_t TotalSentMessages = 0;
            while( TotalSentMessages < MessagesCount)
            {
                TotalSentMessages += Write();
            }
        }
    }

    bool NetworkHandler::IsServer() const
    {
        return ServerConnection == nullptr;
    }

    IConnection* NetworkHandler::FindConnection(const Address& InAddress)
    {
        std::vector<IConnection*>::iterator FoundConnection = std::find_if(Connections.begin(), Connections.end(), [InAddress](const IConnection* element) { return element->GetRemoteEndpoint() == InAddress; });
        return FoundConnection != Connections.end() ? *FoundConnection : nullptr;
    }

    void NetworkHandler::Read()
    {
        Address ReceiveRemoteEndpoint;
        constexpr TIMEVAL TimeoutPoll = { 0,0 };
        while (
            select(0, &ReadWriteSet, NULL, NULL, &TimeoutPoll) > 0 &&
            FD_ISSET(Socket->Handle, &ReadWriteSet) 
        )
        {
            ReadStream.Reset();
            int bytes_read = Socket->ISocket::Receive(ReceiveRemoteEndpoint, ReadStream);

            if (bytes_read <= 0)
            {
                break;
            }

            IConnection* Connection = nullptr;
            if (IsServer() && ReceiveRemoteEndpoint)
            {
                Connection = FindConnection(ReceiveRemoteEndpoint);
                if (!Connection)
                {
                    // New Connection
                    Connection = ConnectionHandlersFunc.OnNewConnection(std::move(ReceiveRemoteEndpoint));
                    if (Connection == nullptr)
                    {
                        // Refused connection
                        continue;
                    }

                    AddConnection(Connection);
                }

                if (!Connection->IsConnected())
                {
                    printf("Connection %u timed out!", RemoveConnection(Connection));
                    ConnectionHandlersFunc.OnDeleteConnection(Connection);
                    continue;
                }
            }

            if (bytes_read > ProtocolHeaderSizeLessId)
            {
                if (Connection->OnPacketReceived(ReadStream))
                {
                    // flag last packet needing a ack
                    Connection->SetNeedsAck();
                }
            }
        }
    }

    int32_t NetworkHandler::Write()
    {
        constexpr TIMEVAL TimeoutPoll = { 0,0 };
        int NbSocketRdy = select(0, NULL, &ReadWriteSet, NULL, &TimeoutPoll);
        if (NbSocketRdy == SOCKET_ERROR)
        {
            printf("select() returned with error %d\n", WSAGetLastError());
            return 0;
        }

        int32_t MessageCount = 0;
        if (NbSocketRdy > 0 &&
            FD_ISSET(Socket->Handle, &ReadWriteSet))
        {
            if (IsServer())
            {
                for (IConnection* Connection : Connections)
                {
                    MessageCount += WriteOnConnection(Connection);
                }
            }
            else
            {
                MessageCount = WriteOnConnection(ServerConnection);
            }
        }

        return MessageCount;
    }

    int32_t NetworkHandler::WriteOnConnection(IConnection* InConnection)
    {
        int32_t MessageCount = 0;
        if (InConnection && InConnection->OnPacketSent(WriteStream, MessageCount) )
        {
            if (!Socket->Send(WriteStream, InConnection->GetRemoteEndpoint()))
            {
                printf("Unable to send packet");
            }            
        }

        return MessageCount;
    }