
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

#include <gtest/gtest.h>

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

    EXPECT_EQ(Writer.GetDataSize(), Reader.GetDataSize());
    EXPECT_EQ(memcmp(Writer.GetData(), Reader.GetData(), Writer.GetDataSize()), 0);
    EXPECT_EQ(OgTest, ReadTest);
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

    EXPECT_EQ(Writer.GetDataSize(), Reader.GetDataSize());
    EXPECT_EQ(memcmp(Writer.GetData(), Reader.GetData(), Writer.GetDataSize()), 0);
    EXPECT_EQ(strcmp(OgTest, ReadTest),0);
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

    EXPECT_EQ(Writer.GetDataSize(), Reader.GetDataSize());
    EXPECT_EQ(memcmp(Writer.GetData(), Reader.GetData(), Writer.GetDataSize()), 0);
    EXPECT_EQ(OgTest, ReadTest);
}

TEST(Serialization, SerializeBuffer)
{
    const int BufferSize = 1018;
    uint8_t* Buffer = new uint8_t[BufferSize];
    for (int idx = 0; idx < BufferSize; ++idx)
    {
        Buffer[idx] = (uint8_t)1 + ((idx) % 254);
    }

    Writer.Reset();
    Reader.Reset();

    uint8_t* OgTest = Buffer;
    uint8_t* ReadTest = new uint8_t[BufferSize];
    memset(ReadTest, 0, sizeof(uint8_t) * BufferSize);

    Writer.Serialize<uint8_t*>(OgTest, BufferSize);
    memcpy((void*)Reader.GetData(), Writer.GetData(), MaxMTUSize);
    Reader.Serialize<uint8_t*>(ReadTest, BufferSize);

    EXPECT_EQ(Writer.GetDataSize(), Reader.GetDataSize());
    EXPECT_EQ(memcmp(Writer.GetData(), Reader.GetData(), Writer.GetDataSize()), 0);
    EXPECT_EQ(memcmp(OgTest, ReadTest, BufferSize), 0);
}

TEST(Serialization, SerializeBufferAlignedOnWord)
{
    const int BufferSize = 1018;
    uint8_t* Buffer = new uint8_t[BufferSize];
    for (int idx = 0; idx < BufferSize; ++idx)
    {
        Buffer[idx] = (uint8_t)1 + ((idx) % 254);
    }

    Writer.Reset();
    Reader.Reset();

    uint8_t* OgTest = Buffer;
    uint8_t* ReadTest = new uint8_t[BufferSize];
    memset(ReadTest, 0, sizeof(uint8_t) * BufferSize);

    uint8_t unalignByte1 = 255;
    uint8_t unalignByte2 = 128;
    Writer.Serialize<uint8_t>(unalignByte1);
    Writer.Serialize<uint8_t>(unalignByte2);

    Writer.Serialize<uint8_t*>(OgTest, BufferSize);
    memcpy((void*)Reader.GetData(), Writer.GetData(), MaxMTUSize);
    uint8_t unalignByteRead1 = 0;
    uint8_t unalignByteRead2 = 0;
    Reader.Serialize<uint8_t>(unalignByteRead1);
    Reader.Serialize<uint8_t>(unalignByteRead2);
    Reader.Serialize<uint8_t*>(ReadTest, BufferSize);

    EXPECT_EQ(Writer.GetDataSize(), Reader.GetDataSize());
    EXPECT_EQ(memcmp(Writer.GetData(), Reader.GetData(), Writer.GetDataSize()), 0);
    EXPECT_EQ(unalignByte1, unalignByteRead1);
    EXPECT_EQ(unalignByte2, unalignByteRead2);
    EXPECT_EQ(memcmp(OgTest, ReadTest, BufferSize),0);
}


TEST(Serialization, SerializeBufferNotAligned)
{
    const int BufferSize = 1018;
    uint8_t* Buffer = new uint8_t[BufferSize];
    for (int idx = 0; idx < BufferSize; ++idx)
    {
        Buffer[idx] = (uint8_t)1 + ((idx) % 254);
    }

    Writer.Reset();
    Reader.Reset();

    uint8_t* OgTest = Buffer;
    uint8_t* ReadTest = new uint8_t[BufferSize];
    memset(ReadTest, 0, sizeof(uint8_t) * BufferSize);

    uint8_t unalignByte1 = 4;
    Writer.Serialize<uint8_t>(unalignByte1, { 8 });

    Writer.Serialize<uint8_t*>(OgTest, BufferSize);
    memcpy((void*)Reader.GetData(), Writer.GetData(), MaxMTUSize);
    uint8_t unalignByteRead1 = 0;
    uint8_t unalignByteRead2 = 0;
    Reader.Serialize<uint8_t>(unalignByteRead1, { 8 });
    Reader.Serialize<uint8_t*>(ReadTest, BufferSize);

    EXPECT_EQ(Writer.GetDataSize(), Reader.GetDataSize());
    EXPECT_EQ(memcmp(Writer.GetData(), Reader.GetData(), Writer.GetDataSize()), 0);
    EXPECT_EQ(unalignByte1, unalignByteRead1);
    EXPECT_EQ(memcmp(OgTest, ReadTest, BufferSize), 0);
}

TEST(Serialization, SerializeVector2f)
{
    Writer.Reset();
    Reader.Reset();
    NetObjectManager::Get().Register<Vector2f>();

    Vector2f MyOgVector(25.123f, 96.456f);
    Vector2f MyReadVector;

    MyOgVector.Serialize(Writer);
    memcpy((void*)Reader.GetData(), Writer.GetData(), MaxMTUSize);
    MyReadVector.Serialize(Reader);

    EXPECT_EQ(Writer.GetDataSize(),Reader.GetDataSize()) << "Expecting Writer and Reader buffer to be the same size";
    EXPECT_EQ(memcmp(Writer.GetData(), Reader.GetData(), Writer.GetDataSize()), 0) << "Expecting Writer and Reader buffer to have the same content";
    EXPECT_EQ(MyOgVector, MyReadVector);
}

TEST(Serialization, SerializeBool)
{
    UnitTest_SerializePrimitives(true);
}
TEST(Serialization, SerializeUInt8)
{
    UnitTest_SerializePrimitives<uint8_t>(8);
}
TEST(Serialization, SerializeUInt16)
{
    UnitTest_SerializePrimitives<uint16_t>(16);
}
TEST(Serialization, SerializeUInt32)
{
    UnitTest_SerializePrimitives<uint32_t>(32);
}
TEST(Serialization, SerializeUInt64)
{
    UnitTest_SerializePrimitives<uint64_t>(64);
}
TEST(Serialization, SerializeInt8)
{
    UnitTest_SerializePrimitives<int8_t>(-8);
}
TEST(Serialization, SerializeInt16)
{
    UnitTest_SerializePrimitives<int16_t>(-16);
}
TEST(Serialization, SerializeInt32)
{
    UnitTest_SerializePrimitives<int32_t>(-32);
}
TEST(Serialization, SerializeInt64)
{
    UnitTest_SerializePrimitives<int64_t>(-64);
}
TEST(Serialization, SerializeFloat)
{
    UnitTest_SerializePrimitives<float>(123.456f);
}
TEST(Serialization, SerializeDouble)
{
    UnitTest_SerializePrimitives<double>(987654.321);
}
TEST(Serialization, SerializeStdString)
{
    std::string Test("TestString");
    UnitTest_SerializePrimitives(Test);
}
TEST(Serialization, SerializeCString)
{
    UnitTest_SerializePrimitives("TestString\0");
}
TEST(Serialization, SerializeEnum)
{
    UnitTest_SerializePrimitives(EnumTest::B);
}

//Client: 127.0.0.1:30000 300001
//Server: 30000
/*DEFINE_UNIT_TEST(Serialization)
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
    const int BufferSize = 1018;
    uint8_t* Buffer = new uint8_t[BufferSize];
    for (int idx = 0; idx < BufferSize; ++idx)
    {
        Buffer[idx] = (uint8_t) 1 + (( idx) % 254);
    }
    UnitTest_SerializeBuffer(Buffer, BufferSize);
    UnitTest_SerializeBufferAlignedOnWord(Buffer, BufferSize);
    UnitTest_SerializeBufferNotAligned(Buffer, BufferSize);

    return true;
}*/