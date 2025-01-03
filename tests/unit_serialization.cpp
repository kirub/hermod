
#include "unit_serialization.h"
#include <hermod/platform/Platform.h>
#include <hermod/socket/UDPSocket.h>
#include <hermod/socket/Address.h>
#include <hermod/protocol/Connection.h>
#include <hermod/utilities/Utils.h>
#include <hermod/replication/NetProperty.h>
#include <hermod/serialization/WriteStream.h>
#include <hermod/serialization/ReadStream.h>
#include "Vector2f.h"

static const bool DISABLE_TIMEOUT = true;

serialization::WriteStream Writer(MaxMTUSize);
serialization::ReadStream Reader(MaxMTUSize);

template < typename T>
void UnitTest_SerializePrimitives(T TestValue)
{
    Writer.Reset();
    Reader.Reset();

    T OgTest(TestValue);
    T ReadTest;

    Writer.Serialize(OgTest);
    memcpy((void*)Reader.GetData(), Writer.GetData(), MaxMTUSize);
    Reader.Serialize(ReadTest);

    assert(Writer.GetDataSize() == Reader.GetDataSize());
    assert(memcmp(Writer.GetData(), Reader.GetData(), Writer.GetDataSize()) == 0);
    assert(OgTest == ReadTest);
}

template <>
void UnitTest_SerializePrimitives<const char*>(const char* TestValue)
{
    Writer.Reset();
    Reader.Reset();

    std::size_t StringLen = strlen(TestValue);
    char* OgTest = new char[StringLen+1];
    memcpy(OgTest, TestValue, sizeof(char) * StringLen);
    OgTest[StringLen] = '\0';
    char* ReadTest = new char[StringLen+1];
    memset(ReadTest, 0, sizeof(char) * (StringLen + 1));

    Writer.Serialize<char*>(OgTest, StringLen );
    memcpy((void*)Reader.GetData(), Writer.GetData(), MaxMTUSize);
    Reader.Serialize<char*>(ReadTest, StringLen);

    assert(Writer.GetDataSize() == Reader.GetDataSize());
    assert(memcmp(Writer.GetData(), Reader.GetData(), Writer.GetDataSize()) == 0);
    assert(strcmp(OgTest, ReadTest)==0);
}

template <>
void UnitTest_SerializePrimitives<std::string>(std::string TestValue)
{
    Writer.Reset();
    Reader.Reset();

    std::size_t StringLen = TestValue.length();
    std::string OgTest(TestValue);
    std::string ReadTest;
    ReadTest.clear();


    Writer.Serialize<std::string>(OgTest, StringLen);
    memcpy((void*)Reader.GetData(), Writer.GetData(), MaxMTUSize);
    Reader.Serialize<std::string>(ReadTest, StringLen);

    assert(Writer.GetDataSize() == Reader.GetDataSize());
    assert(memcmp(Writer.GetData(), Reader.GetData(), Writer.GetDataSize()) == 0);
    assert(OgTest == ReadTest);
}

void UnitTest_SerializeBuffer(uint8_t* Buffer, std::size_t BufferSize)
{
    Writer.Reset();
    Reader.Reset();

    uint8_t* OgTest = Buffer;
    uint8_t* ReadTest = new uint8_t[BufferSize];
    memset(ReadTest, 0, sizeof(uint8_t) * BufferSize);

    Writer.Serialize<uint8_t*>(OgTest, BufferSize);
    memcpy((void*)Reader.GetData(), Writer.GetData(), MaxMTUSize);
    Reader.Serialize<uint8_t*>(ReadTest, BufferSize);

    assert(Writer.GetDataSize() == Reader.GetDataSize());
    assert(memcmp(Writer.GetData(), Reader.GetData(), Writer.GetDataSize()) == 0);
    memcmp(OgTest, ReadTest, BufferSize);
}

void UnitTest_SerializeVector2f()
{
    Writer.Reset();
    Reader.Reset();
    NetObjectManager::Get().Register<Vector2f>();

    Vector2f MyOgVector(25.123f, 96.456f);
    Vector2f MyReadVector;

    MyOgVector.Serialize(Writer);
    memcpy((void*)Reader.GetData(), Writer.GetData(), MaxMTUSize);
    MyReadVector.Serialize(Reader);

    assert(Writer.GetDataSize() == Reader.GetDataSize());
    assert(memcmp(Writer.GetData(), Reader.GetData(), Writer.GetDataSize()) == 0);
    assert(MyOgVector == MyReadVector);
}

//Client: 127.0.0.1:30000 300001
//Server: 30000
DEFINE_UNIT_TEST(Serialization)
{        
    location = std::source_location::current();

    UnitTest_SerializePrimitives(true);
    UnitTest_SerializePrimitives<uint8_t>(8);
    UnitTest_SerializePrimitives<uint16_t>(16);
    UnitTest_SerializePrimitives<uint32_t>(32);
    UnitTest_SerializePrimitives<uint64_t>(64);
    UnitTest_SerializePrimitives<int8_t>(-8);
    UnitTest_SerializePrimitives<int16_t>(-16);
    UnitTest_SerializePrimitives<int32_t>(-32);
    UnitTest_SerializePrimitives<int64_t>(-64);
    UnitTest_SerializePrimitives<float>(123.456f);
    UnitTest_SerializePrimitives<double>(987654.321);
    std::string Test("TestString");
    UnitTest_SerializePrimitives("TestString\0");
    UnitTest_SerializePrimitives(Test);
    UnitTest_SerializePrimitives(Test::B);
    UnitTest_SerializeVector2f();
    const int BufferSize = 1017;
    uint8_t* Buffer = new uint8_t[BufferSize];
    for (int idx = 0; idx < BufferSize; ++idx)
    {
        Buffer[idx] = (uint8_t) 1 + (( idx) % 254);
    }
    UnitTest_SerializeBuffer(Buffer, BufferSize);

    return true;
}