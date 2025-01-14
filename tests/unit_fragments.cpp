#include "unit_fragments.h"
#include <hermod/replication/NetObjectInterface.h>
#include <hermod/protocol/Connection.h>

class MockBigPacket
    : public proto::INetObject
{
    CLASS_ID(MockBigPacket)
public:

    enum Filling
    {
        None,
        Filled,
        LastPacketHalf
    };

    MockBigPacket(int NumberOfFragments, Filling FillWithData = Filled)
        : NumberOfFrags(NumberOfFragments)
        , bufferSize(0)
        , buffer( nullptr )
    {
        Init(NumberOfFragments);
        if (FillWithData)
        {
            Fill(FillWithData);
        }
    }
    MockBigPacket(const MockBigPacket& ToCopy)
        : NumberOfFrags(ToCopy.NumberOfFrags)
        , bufferSize(ToCopy.bufferSize)
        , buffer(nullptr)
    {
        Init(NumberOfFrags);
        memcpy(buffer, ToCopy.buffer, bufferSize);
    }

    virtual ~MockBigPacket()
    {
        delete[] buffer;
    }

    void Init(int NumberOfFragments)
    {
        assert(NumberOfFragments > 0);
        NumberOfFrags = NumberOfFragments;
        bufferSize = (MaxFragmentSize - sizeof(uint32_t) - 2) + (NumberOfFragments - 1) * MaxFragmentSize; // because we want exactly 2 fragments and serialize packets have a packet id (uint32_t) overhead + raw buffer has a 2 bytes overhead
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

    void Fill(Filling FillWithData)
    {
        static std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";

        unsigned int StopAt = FillWithData == Filled ? bufferSize : bufferSize - (MaxFragmentSize / 2);
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

static const int bufferSize = MaxMTUSize;
struct MockUDPPacket
{
    int PacketSize;
    uint8_t buffer[bufferSize];

    MockUDPPacket(uint8_t* InBuffer, int InSize)
        : PacketSize(InSize)
    {
        assert(bufferSize >= InSize);
        memcpy(buffer, InBuffer, std::min(InSize, bufferSize));
    }

    int Read(uint8_t* data, int MaxSize)
    {
        assert(MaxSize >- PacketSize);
        int ReceiveBytesCount = std::min(PacketSize, MaxSize);
        memcpy(data, buffer, ReceiveBytesCount);
        return ReceiveBytesCount;
    }
};

class MockSocket
    : public ISocket
{
    Address SendAddr;
    std::queue<MockUDPPacket> Packets;

public:

    MockSocket(unsigned short port)
    {
    }

    std::size_t DataAvailable()
    {
        return Packets.size();
    }

    virtual bool Send(const unsigned char* data, int len, const Address& dest) override
    {
        Packets.push(MockUDPPacket( (uint8_t*)data, len ));
        SendAddr = dest;
        return true;
    }
    virtual int Receive(Address& sender, unsigned char* data, int len) override
    { 
        int ReceiveBytesCount = 0;
        if (len > 0 && DataAvailable() > 0)
        {
            sender = SendAddr;
            MockUDPPacket& Packet = Packets.front();
            ReceiveBytesCount = Packet.Read(data, len);
            Packets.pop();
        }
        return ReceiveBytesCount;
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
    uint8_t NumberOfFragments = FragmentCount;
    NetObjectManager::Get().Register<MockBigPacket>([NumberOfFragments]() { return new MockBigPacket(NumberOfFragments, MockBigPacket::None); });
    srand((unsigned int)time(NULL));
    bool ReceivedEqualSent = false;
    TestableConnection Connection(3000, 10000);

    MockBigPacket Packet(FragmentCount);

    assert(Connection.Send(Packet));
    MockBigPacket SentPacket = Packet;
    Packet.Reset();

    Connection.OnReceiveObject(std::bind(OnReceiveObject, std::placeholders::_1, ReceivedEqualSent, SentPacket));

    Connection.Update(33.3f);
    NetObjectManager::Get().Unregister<MockBigPacket>();
}

template <uint8_t FragmentCount>
void UnitTest_SendImcompleteFragmented()
{
    uint8_t NumberOfFragments = FragmentCount;
    NetObjectManager::Get().Register<MockBigPacket>([NumberOfFragments]() { return new MockBigPacket(NumberOfFragments, MockBigPacket::None); });
    srand((unsigned int)time(NULL));
    bool ReceivedEqualSent = false;
    TestableConnection Connection(3000, 10000);

    MockBigPacket Packet(FragmentCount, MockBigPacket::LastPacketHalf);

    assert(Connection.Send(Packet));
    MockBigPacket SentPacket = Packet;
    Packet.Reset();

    Connection.OnReceiveObject(std::bind(OnReceiveObject, std::placeholders::_1, ReceivedEqualSent, SentPacket));

    Connection.Update(33.3f);
    NetObjectManager::Get().Unregister<MockBigPacket>();
}

//Client: 127.0.0.1:30000 300001
//Server: 30000
DEFINE_UNIT_TEST(Fragments)
{        
    REGISTER_LOCATION;

    proto::Fragment* test = new proto::Fragment();
    delete test;

    UnitTest_SendFragmented<1>();
    UnitTest_SendFragmented<2>();
    UnitTest_SendFragmented<4>();
    UnitTest_SendFragmented<10>();

    UnitTest_SendImcompleteFragmented<1>();
    UnitTest_SendImcompleteFragmented<2>();
    UnitTest_SendImcompleteFragmented<4>();
    UnitTest_SendImcompleteFragmented<10>();

    return true;
}