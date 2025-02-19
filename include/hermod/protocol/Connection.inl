#include <hermod/protocol/Connection.h>
#include <hermod/protocol/Fragment.h>
#include <hermod/replication/NetObjectInterface.h>
#include <hermod/replication/NetObjectManager.h>
#include <hermod/socket/Address.h>
#include <hermod/utilities/Utils.h>

#include <iostream>

template < TSocket SocketType, TProtocol ProtocolType>
Connection<SocketType, ProtocolType>::Connection(unsigned short InboundPort, TimeMs InConnectionTimeoutMs)
    : Connection(Address(), InboundPort, InConnectionTimeoutMs)
{
}

template < TSocket SocketType, TProtocol ProtocolType>
Connection<SocketType, ProtocolType>::Connection(Address RemoteEndpoint, unsigned short InboundPort, TimeMs InConnectionTimeoutMs)
    : Socket(std::make_unique<SocketType>(InboundPort))
    , ConnectionTimeoutSec(InConnectionTimeoutMs)
    , LastPacketReceiveTimeout(InConnectionTimeoutMs)
    , RemoteEndpoint(RemoteEndpoint)
    , IsServerConnection(!RemoteEndpoint)
    , Writer(MaxMTUSize)
    , Reader(MaxMTUSize)
    , MyProtocol(std::make_unique<ProtocolType>(DefaultProtocolId))
    , PacketsSent()
    , NetObjectQueues({std::make_shared<proto::NetObjectQueue256>(),std::make_shared<proto::NetObjectQueue256>()})
{
    MyProtocol->OnPacketAcked([this](uint16_t InSequenceId) { AckPacketSent(InSequenceId); });
    MyProtocol->OnPacketLost([this](uint16_t InSequenceId) { Resend(InSequenceId); });
}

template < TSocket SocketType, TProtocol ProtocolType>
Connection<SocketType, ProtocolType>::~Connection()
{
    for (proto::NetObjectQueue& Queue : NetObjectQueues)
    {
        Queue.Clear();
    }
}
template < TSocket SocketType, TProtocol ProtocolType>
void Connection<SocketType, ProtocolType>::Resend(uint16_t InPacketId)
{
    /*const int Index = InPacketId % PacketSentHistorySize;
    if (serialization::WriteStream ResendBuffer = PacketsSent[Index])
    {
        Writer.Reset();

        MyProtocol->Serialize(Writer);
        memcpy((void*)Writer.GetData(), ResendBuffer.GetData(), ResendBuffer.GetDataSize() );

        constexpr bool IsResend = true;
        Send(Writer, Reliable, IsResend);
    }*/
}

template < TSocket SocketType, TProtocol ProtocolType>
void Connection<SocketType, ProtocolType>::AckPacketSent(uint16_t InPacketId)
{
    const int Index = InPacketId % PacketSentHistorySize;
    PacketsSent[Index].Clear();
}

template < TSocket SocketType, TProtocol ProtocolType>
const unsigned char* Connection<SocketType, ProtocolType>::GetData()
{
    return Reader.GetData() + MyProtocol->Size();
}

template < TSocket SocketType, TProtocol ProtocolType>
bool Connection<SocketType, ProtocolType>::Send(proto::NetObjectPtr InNetObject)
{
    if (!IsConnected())
    {
        printf(__FUNCTION__ ": Connection to %s lost", std::string(RemoteEndpoint).c_str());
        return false;
    }

    serialization::FakeWriteStream MeasureWriter;
    if( !InNetObject->Serialize(MeasureWriter) )
    {
        return false;
    }
    MeasureWriter.Flush();

    bool HasError = false;
    if (MeasureWriter.GetDataSize() > MaxMTUSize)
    {
        // Handle Packet Fragmentation
        proto::FragmentHandler Fragments(*InNetObject, MeasureWriter.GetDataSize(), MaxFragmentSize);
        while(!Fragments.Entries.empty())
        {
            proto::FragmentHandler::ValueType Fragment = Fragments.Entries.front();
            Fragments.Entries.pop_front();
            if (proto::NetObjectPtr ObjecPtr = std::static_pointer_cast<proto::INetObject>(Fragment))
            {
                NetObjectQueues[SendQueue].AddForSend(ObjecPtr);
            }
        }
    }
    else
    {
        NetObjectQueues[SendQueue].AddForSend(InNetObject);
    }

    return true;
}

template < TSocket SocketType, TProtocol ProtocolType>
bool Connection<SocketType, ProtocolType>::Send(serialization::WriteStream& InStream)
{
    if (Socket->Send(InStream, RemoteEndpoint))
    {
        MyProtocol->OnPacketSent(InStream);
        return true;
    }

    return false;
}

template < TSocket SocketType, TProtocol ProtocolType>
proto::NetObjectPtr Connection<SocketType, ProtocolType>::Receive()
{
    return NetObjectQueues[ReceiveQueue].DequeueObject();
}

template < TSocket SocketType, TProtocol ProtocolType>
void Connection<SocketType, ProtocolType>::OnPacketReceived(serialization::ReadStream& InStream)
{
    LastPacketReceiveTimeout = ConnectionTimeoutSec;

    // Try handle packet
    while (InStream.GetBytesRemaining() > 0)
    {
        OnMessageReceived(InStream);
    }
}

template < TSocket SocketType, TProtocol ProtocolType>
void Connection<SocketType, ProtocolType>::OnMessageReceived(serialization::ReadStream& InStream, uint8_t NetObjectOrderId /*= 255*/, uint8_t NetObjectIdSpaceCount /*= 1*/)
{
    proto::NetObjectPtr NetObject;

    if (NetObjectOrderId == 255)
    {
        NetObjectOrderId = proto::NetObjectQueue::ReadMessageId(InStream);
    }

    uint32_t NetObjectClassId = 0;
    assert(InStream.Serialize(NetObjectClassId));

    if (NetObject = NetObjectManager::Get().Instantiate(NetObjectClassId))
    {
        assert(NetObject->Serialize(InStream, NetObjectManager::Get().GetPropertiesListeners(*NetObject)));
        assert(InStream.Flush());

        if (type::is_a<proto::Fragment>(NetObjectClassId))
        {
            proto::FragmentHandler::ValueType FragmentPtr = std::static_pointer_cast<proto::Fragment>(NetObject);
            FragmentPtr->MessageId = NetObjectOrderId;
            Fragments.OnFragment(FragmentPtr);
            if (Fragments.IsComplete())
            {
                serialization::ReadStream BunchStream = Fragments.Gather();
                OnMessageReceived(BunchStream, Fragments.Entries[0]->MessageId, Fragments.Entries.size());
                Fragments.Reset();
            }
        }
        else
        {
            if (ReceiveObjectCallback)
            {
                ReceiveObjectCallback(*NetObject);
            }
            NetObjectQueues[ReceiveQueue].AddForReceive(NetObjectOrderId, NetObject, NetObjectIdSpaceCount);
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

template < TSocket SocketType, TProtocol ProtocolType>
bool Connection<SocketType, ProtocolType>::Send(unsigned char* Data, std::size_t Len)
{
    if (!IsConnected())
    {
        printf(__FUNCTION__ ": Connection to %s lost", std::string(RemoteEndpoint).c_str());
        return false;
    }

    Writer.Reset();

    MyProtocol->Serialize(Writer);

    if (Len > 0)
    {
        uint32_t PacketId = proto::INetObject::CustomDataId;
        Writer.Serialize(PacketId); // Custom data with no packet type
        Writer.Serialize(Data, serialization::NetPropertySettings<unsigned char*>(Len));
    }

    if (Socket->Send(Writer, RemoteEndpoint))
    {
        printf("SEND: %s\n", Data);
        MyProtocol->OnPacketSent(Writer);
        return true;
    }

    return false;
}

template < TSocket SocketType, TProtocol ProtocolType>
bool Connection<SocketType, ProtocolType>::Flush()
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

template < TSocket SocketType, TProtocol ProtocolType>
bool Connection<SocketType, ProtocolType>::IsConnected() const
{
    return LastPacketReceiveTimeout >= 0;
}

template < TSocket SocketType, TProtocol ProtocolType>
bool Connection<SocketType, ProtocolType>::IsClient() const
{
    return !IsServerConnection;
}

template < TSocket SocketType, TProtocol ProtocolType>
bool Connection<SocketType, ProtocolType>::IsServer() const
{
    return IsServerConnection;
}

template < TSocket SocketType, TProtocol ProtocolType>
IConnection::Error Connection<SocketType, ProtocolType>::Update(TimeMs TimeDelta)
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
         
                if (Reader.GetBytesRemaining() >= sizeof(uint32_t))
                {
                    OnPacketReceived(Reader);

                    // Ack this packet
                    // TODO: Queue packets in case we send something useful while needing to ack packet
                    Send(nullptr, 0);

                    assert(Reader.Align(32)); // Have to align on word in case another packet is following (corresponding to send's EndStream)
                }

            }
            Reader.Reset();
        }
    }

    return None;
}


template < TSocket SocketType, TProtocol ProtocolType>
proto::NetObjectQueue256& Connection<SocketType, ProtocolType>::GetNetObjectQueue(ObjectQueueType InQueueType)
{
    return NetObjectQueues[InQueueType];
}

template < TSocket SocketType, TProtocol ProtocolType>
const Address& Connection<SocketType, ProtocolType>::GetRemoteEndpoint() const
{
    return RemoteEndpoint;
}