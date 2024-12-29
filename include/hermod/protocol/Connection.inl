#include <hermod/protocol/Connection.h>
#include <hermod/protocol/Fragment.h>
#include <hermod/replication/NetObjectInterface.h>
#include <hermod/replication/NetObjectManager.h>
#include <hermod/socket/Address.h>
#include <hermod/utilities/Utils.h>

#include <iostream>

template < TSocket SocketType>
Connection<SocketType>::Connection(unsigned short InboundPort, TimeMs InConnectionTimeoutMs)
    : Connection(Address(), InboundPort, InConnectionTimeoutMs)
{
}

template < TSocket SocketType>
Connection<SocketType>::Connection(Address RemoteEndpoint, unsigned short InboundPort, TimeMs InConnectionTimeoutMs)
    : Socket(std::make_unique<SocketType>(InboundPort))
    , ConnectionTimeoutSec(InConnectionTimeoutMs)
    , LastPacketReceiveTimeout(InConnectionTimeoutMs)
    , RemoteEndpoint(RemoteEndpoint)
    , IsServerConnection(!RemoteEndpoint)
    , Writer(MaxStreamSize)
    , Reader(MaxPacketSize)
    , MyProtocol(std::make_unique<Protocol>(DefaultProtocolId))
{
    MyProtocol->OnPacketAcked(
        [](uint16_t InPacketId)
        {
            printf("Ack: %u\n", InPacketId);
        });
}


template < TSocket SocketType>
const unsigned char* Connection<SocketType>::GetData()
{
    return Reader.GetData() + MyProtocol->Size();
}

template < TSocket SocketType>
bool Connection<SocketType>::Send(proto::INetObject& Packet)
{
    if (!IsConnected())
    {
        printf(__FUNCTION__ ": Connection to %s lost", std::string(RemoteEndpoint).c_str());
        return false;
    }

    if (Writer.GetDataSize() > MaxPacketSize)
    {
        // Handle Packet Fragmentation
        proto::FragmentHandler Fragments(Writer, MaxPacketSize);
        for (proto::FragmentHandler::ValueType Fragment : Fragments.Entries)
        {
            if (Fragment && !Send(*Fragment))
            {
                return false;
            }
        }
    }
    else
    {
        Writer.Reset();

        MyProtocol->Serialize(Writer);
        Packet.Serialize(Writer);
        if (Socket->Send(Writer, RemoteEndpoint))
        {
            MyProtocol->OnPacketSent(Writer);
            return true;
        }
    }

    return true;
}

template < TSocket SocketType>
bool Connection<SocketType>::Send(unsigned char* Data, std::size_t Len)
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

    if (Socket->Send(Writer, RemoteEndpoint))
    {
        printf("SEND: %s\n", Data);
        MyProtocol->OnPacketSent(Writer);
        return true;
    }

    return false;
}

template < TSocket SocketType>
bool Connection<SocketType>::Flush()
{
    if (!IsConnected())
    {
        printf(__FUNCTION__ ": Connection to %s lost", std::string(RemoteEndpoint).c_str());
        return false;
    }

    // Do something
    
    Writer.Reset();

    return false;
}

template < TSocket SocketType>
void Connection<SocketType>::OnPacketReceived(serialization::ReadStream InStream)
{
    // Try handle packet
    proto::INetObject* NetObject = NetObjectManager::Get().HandlePacket(InStream);
    if (!NetObject)
    {
        // Custom data
        (*ReceiveDataCallback)((unsigned char*)InStream.GetData(), InStream.GetDataSize());
    }
    else if (type::is_a<proto::Fragment, proto::INetObject>(*NetObject))
    {
        Fragments.OnFragment(dynamic_cast<proto::Fragment*>(NetObject));
        if (Fragments.IsComplete())
        {
            OnPacketReceived(Fragments.Gather());
            Fragments.Reset();
        }
    }

}

template < TSocket SocketType>
bool Connection<SocketType>::IsConnected() const
{
    return LastPacketReceiveTimeout >= 0;
}

template < TSocket SocketType>
bool Connection<SocketType>::IsClient() const
{
    return !IsServerConnection;
}

template < TSocket SocketType>
bool Connection<SocketType>::IsServer() const
{
    return IsServerConnection;
}

template < TSocket SocketType>
IConnection::Error Connection<SocketType>::Update(TimeMs TimeDelta)
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
    Reader.Reset();
    int bytes_read = Socket->Receive(Sender, Reader);
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

        if (!MyProtocol->Serialize(Reader))
        {
            printf(__FUNCTION__ ": Invalid header. Discard!\n");
            return InvalidHeader;
        }

        LastPacketReceiveTimeout = ConnectionTimeoutSec;

        OnPacketReceived(Reader);
    }

    return None;
}