#include "unit_fragments.h"
#include <hermod/replication/NetObjectInterface.h>
#include <hermod/protocol/Connection.h>

class MockBigPacket
    : public proto::INetObject
{
public:

    MockBigPacket(int NumberOfFragments)
        : NumberOfFrags(NumberOfFragments)
        , bufferSize(0)
        , buffer( nullptr )
    {
        Init(NumberOfFragments);
        FillWithData();
        NetObjectManager::Get().Register<MockBigPacket>([NumberOfFragments]() { return new MockBigPacket(NumberOfFragments); });
    }
    MockBigPacket(const MockBigPacket& ToCopy)
        : NumberOfFrags(ToCopy.NumberOfFrags)
        , bufferSize(ToCopy.bufferSize)
        , buffer(new uint8_t[bufferSize])
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
        NumberOfFrags = NumberOfFragments;
        bufferSize = MaxPacketSize * NumberOfFragments;
        if (bufferSize > 0)
        {
            if (buffer)
            {
                delete[] buffer;
            }
            buffer = new uint8_t[bufferSize];
        }
    }

    void FillWithData()
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

    virtual bool Serialize(serialization::IStream& Stream, std::optional<NetObjectManager::PropertiesListenerContainer> Mapper = std::optional<NetObjectManager::PropertiesListenerContainer>())
    {
        return Stream.Serialize(buffer, { bufferSize });        
    }

    using OnReceivedCallbackType = std::function<void(const MockBigPacket&)>;
    void SetOnReceivedCallback(const OnReceivedCallbackType& CB)
    {
        Callback = CB;
    }
    virtual void OnReceived() 
    {
        Callback(*this);
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
    static const int bufferSize = 1450;
    uint8_t buffer[bufferSize];

public:

    MockSocket(unsigned short port)
    {
        memset(buffer, 0, sizeof(uint8_t) * bufferSize);
    }

    void Reset()
    {
        memset(buffer, 0, sizeof(uint8_t) * bufferSize);
    }

    virtual bool Send(const unsigned char* data, int len, const Address& dest) override
    {
        assert(bufferSize < len);
        memcpy(buffer, data, len);
        SendBufferSize = len;
        SendAddr = dest;
        return true;
    }
    virtual int Receive(Address& sender, unsigned char* data, int len) override
    { 
        if (len > 0)
        {
            assert(len < SendBufferSize);
            sender = SendAddr;
            memcpy(data, buffer, SendBufferSize);

            Reset();
        }
        return SendBufferSize;
    }
};

using TestableConnection = Connection<MockSocket>;

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

    Packet.SetOnReceivedCallback(
        [&ReceivedEqualSent, SentPacket](const MockBigPacket& ReceivedPacket)
        {
            ReceivedEqualSent = ReceivedPacket == SentPacket;
        }
    );

    Connection.Update(33.3f);

    assert(ReceivedEqualSent);
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