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
    , PacketsSent()
{
    MyProtocol->OnPacketAcked(CallableWithOwner< Connection<SocketType>, void, uint16_t >(*this, &Connection<SocketType>::AckPacketSent));
    MyProtocol->OnPacketLost(CallableWithOwner< Connection<SocketType>, void, uint16_t >(*this, &Connection<SocketType>::Resend));
}

template < TSocket SocketType>
void Connection<SocketType>::Resend(uint16_t InPacketId)
{
    const int Index = InPacketId % PacketSentHistorySize;
    if (serialization::WriteStream ResendBuffer = PacketsSent[Index])
    {
        Send(ResendBuffer, Unreliable);
    }
}

template < TSocket SocketType>
void Connection<SocketType>::AckPacketSent(uint16_t InPacketId)
{
    const int Index = InPacketId % PacketSentHistorySize;
    PacketsSent[Index].Clear();
}

template < TSocket SocketType>
const unsigned char* Connection<SocketType>::GetData()
{
    return Reader.GetData() + MyProtocol->Size();
}

template < TSocket SocketType>
bool Connection<SocketType>::Send(proto::INetObject& Packet, EReliability InReliability)
{
    if (!IsConnected())
    {
        printf(__FUNCTION__ ": Connection to %s lost", std::string(RemoteEndpoint).c_str());
        return false;
    }

    serialization::WriteStream BunchWriter(MaxStreamSize);
    uint32_t NetObjectClassId = Packet.GetClassId();
    bool HasError = !(BunchWriter.Serialize(NetObjectClassId) && Packet.SerializeProperties(BunchWriter));
    BunchWriter.EndWrite();

    if (!HasError)
    {
        if (BunchWriter.GetDataSize() > MaxMTUSize)
        {
            // Handle Packet Fragmentation
            proto::FragmentHandler Fragments(BunchWriter, MaxFragmentSize);
            for (proto::FragmentHandler::ValueType Fragment : Fragments.Entries)
            {
                if (Fragment)
                {
                    if (!Send(*Fragment, InReliability))
                    {
                        return false;
                    }
                }
            }
        }
        else
        {
            Writer.Reset();

            MyProtocol->Serialize(Writer);

            uint32_t NetObjectClassId = Packet.GetClassId();
            bool HasError = !(Writer.Serialize(NetObjectClassId) && Packet.Serialize(Writer));

            Writer.EndWrite();

            HasError = HasError || Send(Writer, InReliability);
        }
    }

    return !HasError;
}

template < TSocket SocketType>
bool Connection<SocketType>::Send(serialization::WriteStream& Stream, EReliability InReliability)
{
    if (Socket->Send(Writer, RemoteEndpoint))
    {
        uint16_t PacketSequenceId = MyProtocol->OnPacketSent(Writer);
        if (InReliability == Reliable)
        {
            const int Index = PacketSequenceId % PacketSentHistorySize;
            PacketsSent[Index] = Writer;
        }

        return true;
    }

    return false;
}


template < TSocket SocketType>
void Connection<SocketType>::OnPacketReceived(serialization::ReadStream& InStream)
{
    // Try handle packet
    bool HasError = false;
    proto::INetObject* NetObject = nullptr;

    uint32_t NetObjectClassId = 0;
    HasError = !InStream.Serialize(NetObjectClassId);

    if (NetObject = NetObjectManager::Get().Instantiate(NetObjectClassId))
    {
        assert(NetObject->Serialize(InStream, NetObjectManager::Get().GetPropertiesListeners(*NetObject)));

        if (type::is_a<proto::Fragment>(NetObjectClassId))
        {
            Fragments.OnFragment(dynamic_cast<proto::Fragment*>(NetObject));
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
        else
        {
            printf("Unhandle packet %u", NetObjectClassId);
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

        if (Sender != RemoteEndpoint)
        {
            printf(__FUNCTION__ ": Invalid remote address. Discard!\n");
            return InvalidRemoteAddress;
        }

        if (bytes_read > 0)
        {
            // Packet header min size is the size of the packet type   
            while (Reader.GetBytesRemaining() >= sizeof(uint32_t))
            {
                if (!MyProtocol->Serialize(Reader))
                {
                    printf(__FUNCTION__ ": Invalid header. Discard!\n");
                    return InvalidHeader;
                }

                LastPacketReceiveTimeout = ConnectionTimeoutSec;
         
                OnPacketReceived(Reader);

                assert(Reader.Align(32)); // Have to align on word in case another packet is following (corresponding to send's EndStream)
            }
            Reader.Reset();
        }
    }

    return None;
}