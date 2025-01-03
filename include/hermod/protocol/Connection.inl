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
    , Writer(MaxMTUSize)
    , Reader(MaxMTUSize)
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

    serialization::WriteStream BunchWriter(MaxStreamSize);
    Packet.SerializeProperties(BunchWriter);
    BunchWriter.EndWrite();

    if (BunchWriter.GetDataSize() > MaxMTUSize)
    {
        // Handle Packet Fragmentation
        proto::FragmentHandler Fragments(BunchWriter, MaxFragmentSize);
        for (proto::FragmentHandler::ValueType Fragment : Fragments.Entries)
        {
            if (Fragment)
            {
                uint32_t NetObjectClassId = Fragment->GetClassId();
                bool HasError = !Writer.Serialize(NetObjectClassId);

                uint8_t FragmentCount = 0;
                HasError &= Writer.Serialize(Fragment->Count);

                uint8_t FragmentId = 0;
                HasError &= Writer.Serialize(Fragment->Id, { Fragment->Count });

                Writer.Align(32);

                HasError &= Fragment->Serialize(Writer);

                Writer.EndWrite();

                if (HasError || !Socket->Send(Writer, RemoteEndpoint))
                {
                    return false;
                }

                MyProtocol->OnPacketSent(Writer);
            }
        }
    }
    else
    {
        Writer.Reset();

        MyProtocol->Serialize(Writer);

        uint32_t NetObjectClassId = Packet.GetClassId();
        bool HasError = !Writer.Serialize(NetObjectClassId);

        HasError &= Writer.Align(32);

        HasError &= Packet.Serialize(Writer);

        Writer.EndWrite();

        if (!HasError && Socket->Send(Writer, RemoteEndpoint))
        {
            MyProtocol->OnPacketSent(Writer);
        }
    }

    return true;
}


template < TSocket SocketType>
void Connection<SocketType>::OnPacketReceived(serialization::ReadStream& InStream)
{
    // Try handle packet
    bool HasError = false;
    proto::INetObject* NetObject = nullptr;

    uint32_t NetObjectClassId = 0;
    HasError = !InStream.Serialize(NetObjectClassId);

    uint8_t FragmentCount = 0;
    uint8_t FragmentId = 0;
    const bool IsFragment = type::is_a<proto::Fragment>(NetObjectClassId);
    if (IsFragment)
    {
        HasError &= InStream.Serialize(FragmentCount);
        HasError &= InStream.Serialize(FragmentId, { FragmentCount });
    }

    InStream.Align(32);
    // End Header

    if (NetObject = NetObjectManager::Get().Instantiate(NetObjectClassId))
    {
        if (NetObject->Serialize(InStream, NetObjectManager::Get().GetPropertiesListeners(*NetObject)))
        {
            if (IsFragment)
            {
                Fragments.OnFragment(FragmentCount, FragmentId, dynamic_cast<proto::Fragment*>(NetObject));
                if (Fragments.IsComplete())
                {
                    serialization::ReadStream BunchStream = Fragments.Gather();
                    OnPacketReceived(BunchStream);
                    Fragments.Reset();
                }
            }
            else if (ReceiveObjectCallback)
            {
                ReceiveObjectCallback(*NetObject);
            }
        }
    }
    else
    {
        // Custom data
        if (ReceiveDataCallback)
        {
            (*ReceiveDataCallback)((unsigned char*)InStream.GetData(), InStream.GetDataSize());
        }
    }

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
    while (int bytes_read = Socket->Receive(Sender, Reader))
    {
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

            while (Reader.GetBytesRemaining() > 0)
            {
                OnPacketReceived(Reader);
            }

        }
    }

    return None;
}