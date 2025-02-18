#include "MockBigPacket.h"

MockBigPacket::MockBigPacket()
    : MockBigPacket(0)
{
}
MockBigPacket::MockBigPacket(int NumberOfFragments, Filling FillWithData /*= Filled*/)
    : NumberOfFrags(NumberOfFragments)
    , bufferSize(0)
    , buffer(nullptr)
{
    Init(NumberOfFragments);
    if (FillWithData)
    {
        Fill(FillWithData);
    }
}
MockBigPacket::MockBigPacket(const MockBigPacket& ToCopy)
    : NumberOfFrags(ToCopy.NumberOfFrags)
    , bufferSize(ToCopy.bufferSize)
    , buffer(nullptr)
{
    Init(NumberOfFrags);
    if (buffer != nullptr && ToCopy.buffer != nullptr && bufferSize > 0)
    {
        memcpy(buffer, ToCopy.buffer, bufferSize);
    }
}

MockBigPacket::~MockBigPacket()
{
    delete[] buffer;
}

void MockBigPacket::Init(int NumberOfFragments)
{
    if (NumberOfFragments > 0)
    {
        NumberOfFrags = NumberOfFragments;
        //bufferSize = (MaxFragmentSize - ProtocolHeaderSize) + (NumberOfFragments - 1) * MaxFragmentSize; // because we want exactly 2 fragments and serialize packets have a packet id (uint32_t) overhead + raw buffer has a 2 bytes overhead
        bufferSize = (NumberOfFragments * MaxFragmentSize) - ProtocolHeaderSize;
        InitBuffer(bufferSize);
    }
}
void MockBigPacket::Init(std::size_t SizeInBytes)
{
    NumberOfFrags = (int)(SizeInBytes/MaxFragmentSize);
    NumberOfFrags += (SizeInBytes % MaxFragmentSize) > 0 ? 1 : 0;
    bufferSize = (MaxFragmentSize - ProtocolHeaderSize) + (NumberOfFrags - 1) * MaxFragmentSize; // because we want exactly 2 fragments and serialize packets have a packet id (uint32_t) overhead + raw buffer has a 2 bytes overhead
    InitBuffer(SizeInBytes);
}
void MockBigPacket::InitBuffer(std::size_t bufferSize)
{
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

void MockBigPacket::Fill(Filling FillWithData)
{
    static std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890";

    unsigned int StopAt = (unsigned int)(FillWithData == Filled ? bufferSize : bufferSize - (MaxFragmentSize / 2));
    for (unsigned int i = 0; i < bufferSize; i++)
        buffer[i] = charset[rand() % charset.length()];
}

void MockBigPacket::Reset()
{
    memset(buffer, 0, bufferSize);
}

bool MockBigPacket::operator==(const MockBigPacket& Rhs) const
{
    return memcmp(buffer, Rhs.buffer, std::min(bufferSize, Rhs.bufferSize)) == 0;
}

bool MockBigPacket::SerializeImpl(serialization::IStream& Stream)
{
    return Stream.Serialize<uint8_t*>(buffer, { bufferSize });
}

void MockBigPacket::SetOnReceivedCallback(const OnReceivedCallbackType& CB)
{
    Callback = CB;
}

void MockBigPacket::OnReceived()
{
    if (Callback)
    {
        Callback(*this);
    }
}