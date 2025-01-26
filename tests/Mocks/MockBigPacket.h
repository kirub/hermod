#pragma once

#include <hermod/replication/NetObjectInterface.h>

class MockBigPacket
    : public proto::INetObject
{
    CLASS_ID(MockBigPacket)
public:
    using OnReceivedCallbackType = std::function<void(const MockBigPacket&)>;

    enum Filling
    {
        None,
        Filled,
        LastPacketHalf
    };

    MockBigPacket(int NumberOfFragments, Filling FillWithData = Filled);
    MockBigPacket(const MockBigPacket& ToCopy);

    virtual ~MockBigPacket();

    void Init(int NumberOfFragments);
    void Init(std::size_t SizeInBytes);
    void InitBuffer(std::size_t bufferSize);
    void Reset();
    void Fill(Filling FillWithData);
    void SetOnReceivedCallback(const OnReceivedCallbackType& CB);

    virtual bool SerializeImpl(serialization::IStream& Stream) override;
    virtual void OnReceived() override;

    bool operator==(const MockBigPacket& Rhs) const;
private:

    int NumberOfFrags;
    uint8_t* buffer;
    std::size_t bufferSize;

    OnReceivedCallbackType Callback;
};