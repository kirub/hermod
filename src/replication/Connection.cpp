#include <hermod/protocol/Connection.h>

#include <hermod/replication/NetObjectInterface.h>
#include <hermod/replication/NetObjectManager.h>
#include <hermod/socket/Address.h>
#include <hermod/utilities/Utils.h>

#include <iostream>

static const unsigned int MyProtocolId = 666;


void IConnection::OnReceiveData(OnReceiveDataFunctor InReceiveDataCallback)
{
    ReceiveDataCallback = InReceiveDataCallback;
}

Connection::Connection(unsigned short InboundPort, TimeMs InConnectionTimeoutMs)
    : Connection(Address(), InboundPort, InConnectionTimeoutMs)
{
}

Connection::Connection(Address RemoteEndpoint, unsigned short InboundPort, TimeMs InConnectionTimeoutMs)
    : Socket(InboundPort)
    , ConnectionTimeoutSec(InConnectionTimeoutMs)
    , LastPacketReceiveTimeout(InConnectionTimeoutMs)
    , RemoteEndpoint(RemoteEndpoint)
    , IsServerConnection(!RemoteEndpoint)
    , Writer(SendBuffer, MaxPacketSize)
    , Reader(ReceiveBuffer, MaxPacketSize)
    , MyProtocol(std::make_unique<Protocol>(MyProtocolId))
{
    memset(ReceiveBuffer, 0, MaxPacketSize);
    memset(SendBuffer, 0, MaxPacketSize);

    MyProtocol->OnPacketAcked(
        [](uint16_t InPacketId)
        {
            printf("Ack: %u\n", InPacketId);
        });
}


const unsigned char* Connection::GetData() const
{
    return ReceiveBuffer + MyProtocol->Size();
}

bool Connection::Send(proto::INetObject& Packet)
{
    if (!IsConnected())
    {
        printf(__FUNCTION__ ": Connection to %s lost", std::string(RemoteEndpoint).c_str());
        return false;
    }

    Writer.Reset();

    MyProtocol->Serialize(Writer);
    Packet.Serialize(Writer);

    if (Socket.SendTo(Writer, RemoteEndpoint))
    {
        MyProtocol->OnPacketSent(Writer);
        return true;
    }

    return false;
}
bool Connection::Send(unsigned char* Data, std::size_t Len)
{
    if (!IsConnected())
    {
        printf(__FUNCTION__ ": Connection to %s lost", std::string(RemoteEndpoint).c_str());
        return false;
    }

    Writer.Reset();

    MyProtocol->Serialize(Writer);
    uint32_t PacketId = proto::INetObject::CustomDataId;
    Writer.Serialize(PacketId); // Custom data with no packet type
    Writer.Serialize(Data, serialization::NetPropertySettings<unsigned char*>(Len));

    if (Socket.SendTo(Writer, RemoteEndpoint))
    {
        printf("SEND: %s\n", Data);
        MyProtocol->OnPacketSent(Writer);
        return true;
    }

    return false;
}

void Connection::OnPacketReceived(unsigned char* Data, const int Len)
{
    // Try handle packet
    if (!NetObjectManager::Get().HandlePacket(Reader))
    {
        // Custom data
        (*ReceiveDataCallback)(Data, Len);
    }
}

bool Connection::IsConnected() const
{
    return LastPacketReceiveTimeout >= 0;
}

bool Connection::IsClient() const
{
    return !IsServerConnection;
}

bool Connection::IsServer() const
{
    return IsServerConnection;
}


IConnection::Error Connection::Update(TimeMs TimeDelta)
{
    if (RemoteEndpoint)
    {
        if (!IsConnected())
        {
            printf(__FUNCTION__ ": Connection to %s lost\n", std::string(RemoteEndpoint).c_str());
            RemoteEndpoint = Address();
            return DisconnectionError;
        }

        //printf(__FUNCTION__ ": TimeDelta: %f\n", TimeDelta);
        LastPacketReceiveTimeout -= TimeDelta;
    }

    Address Sender;
    int BufferSize = 256;
    memset(ReceiveBuffer, 0, BufferSize);
    int bytes_read = Socket.RecvFrom(Sender, ReceiveBuffer, sizeof(ReceiveBuffer));
    if (IsServer() && !RemoteEndpoint && Sender)
    {
        RemoteEndpoint = Sender;
    }

    if (bytes_read > 0)
    {
        if (Sender != RemoteEndpoint)
        {
            printf(__FUNCTION__ ": Invalid remote address. Discard!\n");
            return InvalidRemoteAddress;
        }

        const unsigned char* Packet = ReceiveBuffer;
        if (!MyProtocol->Serialize(Reader))
        {
            printf(__FUNCTION__ ": Invalid header. Discard!\n");
            return InvalidHeader;
        }

        LastPacketReceiveTimeout = ConnectionTimeoutSec;

        OnPacketReceived(ReceiveBuffer + MyProtocol->Size(), bytes_read - MyProtocol->Size());
    }

    return None;
}