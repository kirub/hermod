#include <hermod/protocol/Connection.h>

#include <hermod/protocol/Connection.h>
#include <hermod/protocol/Fragment.h>
#include <hermod/replication/NetObjectInterface.h>
#include <hermod/replication/NetObjectManager.h>
#include <hermod/socket/Address.h>
#include <hermod/utilities/Utils.h>

#include <iostream>

Connection::Connection(ProtocolPtr InProtocol, TimeMs InConnectionTimeoutMs)
    : Connection(InProtocol, Address(), InConnectionTimeoutMs)
{
}

Connection::Connection(ProtocolPtr InProtocol, Address RemoteEndpoint, TimeMs InConnectionTimeoutMs)
    : ConnectionTimeoutSec(InConnectionTimeoutMs)
    , LastPacketReceiveTimeout(InConnectionTimeoutMs)
    , RemoteEndpoint(RemoteEndpoint)
    , IsServerConnection(!RemoteEndpoint)
    , Writer(MaxMTUSize)
    , Reader(MaxMTUSize)
    , Protocol(InProtocol)
{
}

Connection::~Connection()
{
    for (proto::NetObjectQueue& Queue : NetObjectQueues)
    {
        Queue.Clear();
    }
}

bool Connection::Send(proto::NetObjectPtr InNetObject)
{
    if (!IsConnected())
    {
        printf(__FUNCTION__ ": Connection to %s lost", std::string(RemoteEndpoint).c_str());
        return false;
    }

    serialization::FakeWriteStream MeasureWriter;
    if (!InNetObject->Serialize(MeasureWriter))
    {
        return false;
    }
    MeasureWriter.Flush();

    bool HasError = false;
    if (MeasureWriter.GetDataSize() > MaxMTUSize)
    {
        // Handle Packet Fragmentation
        proto::FragmentHandler Fragments(*InNetObject, MeasureWriter.GetDataSize(), MaxFragmentSize);
        while (!Fragments.Entries.empty())
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

proto::NetObjectPtr Connection::Receive()
{
    return NetObjectQueues[ReceiveQueue].DequeueObject();
}

uint8_t Connection::GetPacketIdx(uint16_t SequenceId) const
{
    return SequenceId % PacketSentHistorySize;
}

bool Connection::OnPacketSent(serialization::WriteStream& InStream, int32_t& MessageIncludedCount)
{
    MessageIncludedCount = 0;

    bool NeedsAck = GetNeedsAckAndReset();
    proto::NetObjectQueue256& NetObjectQueue = GetNetObjectQueue(IConnection::SendQueue);
    if (!NetObjectQueue.Empty() || NeedsAck)
    {
        InStream.Reset();
        Protocol->Serialize(InStream);

        uint16_t SequenceId = Protocol->OnPacketSent(InStream);
        MessageIncludedCount = NetObjectQueue.GetSendBuffer(InStream, MessageIds[GetPacketIdx(SequenceId)]);
        InStream.Flush();
        return true;
    }

    return false;
}


bool Connection::OnPacketReceived(serialization::ReadStream& InStream)
{
    LastPacketReceiveTimeout = ConnectionTimeoutSec;

    // Packet header min size is the size of the packet type   
    if (!Protocol->Serialize(InStream))
    {
        printf(__FUNCTION__ ": Invalid header. Discard!\n");
        return false;
    }

    if (InStream.GetBytesRemaining() >= sizeof(uint32_t))
    {
        // Try handle packet
        while (InStream.GetBytesRemaining() > 0)
        {
            OnMessageReceived(InStream);
        }
    }

    return true;
}


void Connection::OnMessageReceived(serialization::ReadStream& InStream, uint8_t NetObjectOrderId /*= 255*/, uint8_t NetObjectIdSpaceCount /*= 1*/)
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
                int MaxSize = MaxFragmentSize * (int)Fragments.Entries.size();
                serialization::ReadStream BunchStream(MaxSize);
                Fragments.Gather(BunchStream);
                OnMessageReceived(BunchStream, Fragments.Entries[0]->MessageId, (uint8_t)Fragments.Entries.size());
                Fragments.Reset();
            }
        }
        else
        {
            NetObjectQueues[ReceiveQueue].AddForReceive(NetObjectOrderId, NetObject, NetObjectIdSpaceCount);
        }
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


void Connection::Update(TimeMs TimeDelta)
{
    if (RemoteEndpoint)
    {
        if (!IsConnected())
        {
            printf(__FUNCTION__ ": Connection to %s lost\n", std::string(RemoteEndpoint).c_str());
            RemoteEndpoint = Address();
            if (OnDisconnectedFunc)
            {
                OnDisconnectedFunc();
            }
        }

        //printf(__FUNCTION__ ": TimeDelta: %f\n", TimeDelta);
        LastPacketReceiveTimeout -= TimeDelta;
    }
}



proto::NetObjectQueue256& Connection::GetNetObjectQueue(ObjectQueueType InQueueType)
{
    return NetObjectQueues[InQueueType];
}


const Address& Connection::GetRemoteEndpoint() const
{
    return RemoteEndpoint;
}

