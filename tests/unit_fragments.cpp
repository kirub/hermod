#include "unit_fragments.h"
#include <hermod/replication/NetObjectInterface.h>
#include <hermod/protocol/Connection.h>

class MockBigPacket
    : public proto::INetObject
{
    CLASS_ID(MockBigPacket)
public:

    MockBigPacket(int NumberOfFragments, bool FillWithData = true)
        : NumberOfFrags(NumberOfFragments)
        , bufferSize(0)
        , buffer( nullptr )
    {
        Init(NumberOfFragments);
        if (FillWithData)
        {
            Fill();
        }
        NetObjectManager::Get().Register<MockBigPacket>([NumberOfFragments]() { return new MockBigPacket(NumberOfFragments, false); });
    }
    MockBigPacket(const MockBigPacket& ToCopy)
        : NumberOfFrags(ToCopy.NumberOfFrags)
        , bufferSize(ToCopy.bufferSize)
        , buffer(nullptr)
    {
        Init(NumberOfFrags);
        memcpy(buffer, ToCopy.buffer, bufferSize);
        NetObjectManager::Get().Register<MockBigPacket>([NumberOfFragments = ToCopy.NumberOfFrags]() { return new MockBigPacket(NumberOfFragments); });
    }

    virtual ~MockBigPacket()
    {
        delete[] buffer;
    }

    void Init(int NumberOfFragments)
    {
        assert(NumberOfFragments > 0);
        NumberOfFrags = NumberOfFragments;
        bufferSize = (MaxFragmentSize - 2) + (NumberOfFragments - 1) * MaxFragmentSize;
        if (bufferSize > 0)
        {
            if (buffer)
            {
                delete[] buffer;
            }
            buffer = new uint8_t[bufferSize];
            memset(buffer, 0, sizeof(uint8_t) * bufferSize);
        }
    }

    void Fill()
    {
        static std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";

        for (unsigned int i = 0; i < bufferSize; i++)
            buffer[i] = charset[rand() % charset.length()];
    }

    void Reset()
    {
        memset(buffer, 0, bufferSize);
    }

    bool operator==(const MockBigPacket& Rhs) const
    {
        return memcmp(buffer, Rhs.buffer, std::min(bufferSize, Rhs.bufferSize)) == 0;
    }

    virtual bool SerializeImpl(serialization::IStream& Stream) override
    {
        return Stream.Serialize<uint8_t*>(buffer, { bufferSize });
    }

    using OnReceivedCallbackType = std::function<void(const MockBigPacket&)>;
    void SetOnReceivedCallback(const OnReceivedCallbackType& CB)
    {
        Callback = CB;
    }
    virtual void OnReceived() 
    {
        if (Callback)
        {
            Callback(*this);
        }
    }
private:

    int NumberOfFrags;
    uint8_t* buffer;
    std::size_t bufferSize;

    OnReceivedCallbackType Callback;
};

class MockSocket
    : public ISocket
{
    Address SendAddr;
    int SendBufferSize;
    static const int bufferSize = MaxStreamSize;
    uint8_t buffer[bufferSize];
    int RecvBufferHead;

public:

    MockSocket(unsigned short port)
    {
        Reset();
    }

    void Reset()
    {
        SendBufferSize = 0;
        RecvBufferHead = 0;
        memset(buffer, 0, sizeof(uint8_t) * bufferSize);
    }

    int DataAvailable()
    {
        return std::max(0, SendBufferSize - RecvBufferHead);
    }

    virtual bool Send(const unsigned char* data, int len, const Address& dest) override
    {
        assert(bufferSize > SendBufferSize + len);
        memcpy(buffer + SendBufferSize, data, len);
        SendBufferSize += len;
        SendAddr = dest;
        return true;
    }
    virtual int Receive(Address& sender, unsigned char* data, int len) override
    { 
        int DataOnSocket = DataAvailable();
        if (len > 0 && DataOnSocket > 0)
        {
            assert(RecvBufferHead < SendBufferSize);
            sender = SendAddr;
            int ReceiveBytesCount = std::min(SendBufferSize, MaxFragmentSize + ProtocolHeaderSize + FragmentHeaderSize);
            memcpy(data, buffer + RecvBufferHead, ReceiveBytesCount);
            RecvBufferHead = (RecvBufferHead + ReceiveBytesCount) % bufferSize;
        }
        return DataOnSocket;
    }
};

using TestableConnection = Connection<MockSocket>;

void OnReceiveObject(const proto::INetObject& ReceivedPacket, bool& ReceivedEqualSent, const MockBigPacket& SentPacket)
{
    const MockBigPacket& BigPacketReceived = dynamic_cast<const MockBigPacket&>(ReceivedPacket);
    ReceivedEqualSent = BigPacketReceived == SentPacket;

    assert(ReceivedEqualSent);
}


template <uint8_t FragmentCount>
void UnitTest_SendFragmented()
{
    srand((unsigned int)time(NULL));
    bool ReceivedEqualSent = false;
    TestableConnection Connection(3000, 10000);

    MockBigPacket Packet(FragmentCount);

    assert(Connection.Send(Packet));
    MockBigPacket SentPacket = Packet;
    Packet.Reset();

    Connection.OnReceiveObject(std::bind(OnReceiveObject, std::placeholders::_1, ReceivedEqualSent, SentPacket));

    Connection.Update(33.3f);
}

//Client: 127.0.0.1:30000 300001
//Server: 30000
DEFINE_UNIT_TEST(Fragments)
{        
    REGISTER_LOCATION;

    UnitTest_SendFragmented<1>();
    UnitTest_SendFragmented<2>();
    UnitTest_SendFragmented<4>();
    UnitTest_SendFragmented<10>();

    return true;
}