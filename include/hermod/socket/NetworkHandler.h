#pragma once

#include <hermod/platform/Platform.h>
#include <hermod/utilities/Types.h>
#include <hermod/protocol/ProtocolInterface.h>
#include <hermod/socket/SocketInterface.h>
#include <hermod/protocol/ConnectionInterface.h>
#include <hermod/protocol/NetObjectQueue.h>
#include <hermod/socket/Address.h>
#include <hermod/serialization/ReadStream.h>
#include <hermod/serialization/WriteStream.h>
#include <thread>


template < TSocket SocketType, TProtocol ProtocolType>
class NetworkHandler
{
public:
    using ProtocolPtr = std::shared_ptr<ProtocolType>;
    using SocketPtr = std::shared_ptr<SocketType>;

    class ConnectionHandlers
    {
    public:
        using NewConnectionHandlerType = std::function<IConnection* (const Address&)>;
        using DeleteConnectionHandlerType = std::function<void(IConnection*)>;

        ConnectionHandlers()
            : ConnectionHandlers(nullptr, nullptr)
        { }

        ConnectionHandlers(NewConnectionHandlerType InOnNewConnection, DeleteConnectionHandlerType InOnDeleteConnection)
            : OnNewConnectionFunc(InOnNewConnection)
            , OnDeleteConnectionFunc(InOnDeleteConnection)
        {
        }

        IConnection* OnNewConnection(Address InAddress)
        {
            if (!OnNewConnectionFunc)
            {
                printf("No new connection handler provided!");
                return nullptr;
            }

            return OnNewConnectionFunc(std::move(InAddress));
        }
        void OnDeleteConnection(IConnection* InConnection)
        {
            if (OnDeleteConnectionFunc && InConnection)
            {
                OnDeleteConnectionFunc(InConnection);
            }
        }

    private:
        NewConnectionHandlerType OnNewConnectionFunc;
        DeleteConnectionHandlerType OnDeleteConnectionFunc;
    };

    NetworkHandler(ProtocolPtr InProtocol, SocketPtr InSocket)
        : NetworkHandler(nullptr, InProtocol, InSocket)
    {
    }

    NetworkHandler(IConnection* InServerConnection, ProtocolPtr InProtocol, SocketPtr InSocket)
        : ServerConnection(InServerConnection)
        , NetProtocol(InProtocol)
        , Socket(InSocket)
        , WriteStream(MaxMTUSize)
        , ReadStream(MaxMTUSize)
    {
        FD_ZERO(&ReadWriteSet);
        FD_SET(Socket->Handle, &ReadWriteSet);
    }

    ~NetworkHandler()
    {
        for (IConnection* Connection : Connections)
        {
            ConnectionHandlersFunc.OnDeleteConnection(Connection);            
        }
        Connections.clear();
    }

    void AddConnection(IConnection* InConnection)
    {
        Connections.push_back(InConnection);
    }
    int RemoveConnection(IConnection* InConnection)
    {        
        std::vector<IConnection*>::iterator Found = std::find(Connections.begin(), Connections.end(), InConnection);
        int Index = Found - Connections.begin();
        Connections.erase(Found);
        return Index;
    }

    const std::vector<IConnection*>& GetClientConnections() const
    {
        return Connections;
    }

    const IConnection* GetServerConnection() const
    {
        return ServerConnection;
    }

    void SetConnectionHandlers(ConnectionHandlers InNewConnectionHandlers)
    {
        ConnectionHandlersFunc = InNewConnectionHandlers;
    }

    void OnStartFrame()
    {
        Read();
    }

    void OnEndFrame()
    {
        Write();
    }
    void SendAllMessages(IConnection& InConnection)
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
private:

    bool IsServer() const
    {
        return ServerConnection == nullptr;
    }

    IConnection* FindConnection(const Address& InAddress)
    {
        std::vector<IConnection*>::iterator FoundConnection = std::find_if(Connections.begin(), Connections.end(), [InAddress](const IConnection* element) { return element->GetRemoteEndpoint() == InAddress; });
        return FoundConnection != Connections.end() ? *FoundConnection : nullptr;
    }

    uint8_t GetPacketIdx(uint16_t SequenceId) const
    {
        return SequenceId % MessageHistorySize;
    }

    void Read()
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
                // Packet header min size is the size of the packet type   
                if (!NetProtocol->Serialize(ReadStream))
                {
                    printf(__FUNCTION__ ": Invalid header. Discard!\n");
                    continue;
                }

                if (ReadStream.GetBytesRemaining() >= sizeof(uint32_t))
                {
                    Connection->OnPacketReceived(ReadStream);

                    // flag last packet needing a ack
                    Connection->SetNeedsAck();
                }
            }
        }
    }

    int32_t Write()
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

    int32_t WriteOnConnection(IConnection* InConnection)
    {
        int32_t MessageCount = 0;
        if (InConnection)
        {
            bool NeedsAck = InConnection->GetNeedsAckAndReset();
            proto::NetObjectQueue256& NetObjectQueue = InConnection->GetNetObjectQueue(IConnection::SendQueue);
            if (!NetObjectQueue.Empty() || NeedsAck)
            {
                WriteStream.Reset();
                NetProtocol->Serialize(WriteStream);

                uint16_t SequenceId = NetProtocol->GetLatestSequenceId(IProtocol::Local);
                MessageCount = NetObjectQueue.GetSendBuffer(WriteStream, MessageIds[GetPacketIdx(SequenceId)]);
                WriteStream.Flush();
                if (!Socket->ISocket::Send(WriteStream, InConnection->GetRemoteEndpoint()))
                {
                    printf("Unable to send packet");
                }
                NetProtocol->OnPacketSent(WriteStream);
            }
        }

        return MessageCount;
    }

    static const int MessageHistorySize = 256;

    serialization::WriteStream          WriteStream;
    serialization::ReadStream           ReadStream;

    FD_SET ReadWriteSet;

    IConnection*                        ServerConnection;
    ProtocolPtr                         NetProtocol;
    SocketPtr                           Socket;
    std::vector<IConnection*>           Connections;
    std::vector<int>                    MessageIds[MessageHistorySize];

    ConnectionHandlers                  ConnectionHandlersFunc;

    // If Packet Reliability
    static const int PacketSentHistorySize = 64;
    serialization::WriteStream PacketsSent[PacketSentHistorySize];
};