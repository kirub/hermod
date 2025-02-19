#include "Vector2f.h"

#include <hermod/platform/Platform.h>
#include <hermod/socket/UDPSocket.h>
#include <hermod/socket/Address.h>
#include <hermod/protocol/Connection.h>
#include <hermod/utilities/Utils.h>
#include <hermod/replication/NetProperty.h>
#include <hermod/serialization/WriteStream.h>
#include <hermod/serialization/ReadStream.h>

#include <gtest/gtest.h>
#include <hermod/serialization/FakeWriteStream.h>
#include <hermod/serialization/NetTypeTraits.h>

static const bool DISABLE_TIMEOUT = true;

serialization::WriteStream Writer(MaxMTUSize);
serialization::ReadStream Reader(MaxMTUSize);

void UnitTest_SerializePointer(proto::INetObject* TestValue)
{
    Writer.Reset();
    Reader.Reset();

    proto::INetObject* OgTest(TestValue);
    proto::INetObject* ReadTest;

    Writer.Serialize(OgTest);
    memcpy((void*)Reader.GetData(), Writer.GetData(), MaxMTUSize);
    Reader.Serialize(ReadTest);

    EXPECT_EQ(Writer.GetDataSize(), Reader.GetDataSize());
    EXPECT_EQ(memcmp(Writer.GetData(), Reader.GetData(), Writer.GetDataSize()), 0);
    EXPECT_EQ(OgTest, ReadTest);
}

TEST(Serialization, SerializePointer)
{
    Vector2f* vec2f = new Vector2f(1234.564f, -42.5f);
    UnitTest_SerializePointer(vec2f);
    delete vec2f;
}

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

    delete[] OgTest;
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

template < typename T>
void UnitTest_CompareStreamSerializePrimitives(serialization::IStream& Writer1, serialization::IStream& Writer2, T TestValue)
{
    Writer1.Reset();
    Writer2.Reset();

    Writer1.Serialize(TestValue);
    Writer2.Serialize(TestValue);

    EXPECT_EQ(Writer1.GetDataSize(), Writer2.GetDataSize());
}

template <>
void UnitTest_CompareStreamSerializePrimitives<const char*>(serialization::IStream& Writer1, serialization::IStream& Writer2, const char* TestValue)
{
    Writer1.Reset();
    Writer2.Reset();

    std::size_t StringLen = strlen(TestValue);
    char* OgTest = new char[StringLen + 1];
    memcpy(OgTest, TestValue, sizeof(char) * StringLen);
    OgTest[StringLen] = '\0';

    Writer1.Serialize<char*>(OgTest, StringLen);
    Writer2.Serialize<char*>(OgTest, StringLen);

    EXPECT_EQ(Writer1.GetDataSize(), Writer2.GetDataSize());

    delete[] OgTest;
}

template <>
void UnitTest_CompareStreamSerializePrimitives<std::string>(serialization::IStream& Writer1, serialization::IStream& Writer2, std::string TestValue)
{
    Writer1.Reset();
    Writer2.Reset();

    Writer1.Serialize<std::string>(TestValue, TestValue.length());
    Writer2.Serialize<std::string>(TestValue, TestValue.length());

    EXPECT_EQ(Writer1.GetDataSize(), Writer2.GetDataSize());
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
    Writer.Flush();
    memcpy((void*)Reader.GetData(), Writer.GetData(), MaxMTUSize);

    uint32_t NetObjectClassId = 0;
    ASSERT_TRUE(Reader.Serialize(NetObjectClassId));

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


TEST(Serialization_FakeStream, SerializeBool)
{
    serialization::WriteStream Writer1(MaxMTUSize);
    serialization::FakeWriteStream Writer2(MaxMTUSize);

    UnitTest_CompareStreamSerializePrimitives(Writer1, Writer2, true);
}
TEST(Serialization_FakeStream, SerializeUInt8)
{
    serialization::WriteStream Writer1(MaxMTUSize);
    serialization::FakeWriteStream Writer2(MaxMTUSize);

    UnitTest_CompareStreamSerializePrimitives<uint8_t>(Writer1, Writer2, 8);
}
TEST(Serialization_FakeStream, SerializeUInt16)
{
    serialization::WriteStream Writer1(MaxMTUSize);
    serialization::FakeWriteStream Writer2(MaxMTUSize);

    UnitTest_CompareStreamSerializePrimitives<uint16_t>(Writer1, Writer2, 16);
}
TEST(Serialization_FakeStream, SerializeUInt32)
{
    serialization::WriteStream Writer1(MaxMTUSize);
    serialization::FakeWriteStream Writer2(MaxMTUSize);

    UnitTest_CompareStreamSerializePrimitives<uint32_t>(Writer1, Writer2, 32);
}
TEST(Serialization_FakeStream, SerializeUInt64)
{
    serialization::WriteStream Writer1(MaxMTUSize);
    serialization::FakeWriteStream Writer2(MaxMTUSize);

    UnitTest_CompareStreamSerializePrimitives<uint64_t>(Writer1, Writer2, 64);
}
TEST(Serialization_FakeStream, SerializeInt8)
{
    serialization::WriteStream Writer1(MaxMTUSize);
    serialization::FakeWriteStream Writer2(MaxMTUSize);

    UnitTest_CompareStreamSerializePrimitives<int8_t>(Writer1, Writer2, -8);
}
TEST(Serialization_FakeStream, SerializeInt16)
{
    serialization::WriteStream Writer1(MaxMTUSize);
    serialization::FakeWriteStream Writer2(MaxMTUSize);

    UnitTest_CompareStreamSerializePrimitives<int16_t>(Writer1, Writer2, -16);
}
TEST(Serialization_FakeStream, SerializeInt32)
{
    serialization::WriteStream Writer1(MaxMTUSize);
    serialization::FakeWriteStream Writer2(MaxMTUSize);

    UnitTest_CompareStreamSerializePrimitives<int32_t>(Writer1, Writer2, -32);
}
TEST(Serialization_FakeStream, SerializeInt64)
{
    serialization::WriteStream Writer1(MaxMTUSize);
    serialization::FakeWriteStream Writer2(MaxMTUSize);

    UnitTest_CompareStreamSerializePrimitives<int64_t>(Writer1, Writer2, -64);
}
TEST(Serialization_FakeStream, SerializeFloat)
{
    serialization::WriteStream Writer1(MaxMTUSize);
    serialization::FakeWriteStream Writer2(MaxMTUSize);

    UnitTest_CompareStreamSerializePrimitives<float>(Writer1, Writer2, 123.456f);
}
TEST(Serialization_FakeStream, SerializeDouble)
{
    serialization::WriteStream Writer1(MaxMTUSize);
    serialization::FakeWriteStream Writer2(MaxMTUSize);

    UnitTest_CompareStreamSerializePrimitives<double>(Writer1, Writer2, 987654.321);
}
TEST(Serialization_FakeStream, SerializeStdString)
{
    serialization::WriteStream Writer1(MaxMTUSize);
    serialization::FakeWriteStream Writer2(MaxMTUSize);

    std::string Test("TestString");
    UnitTest_CompareStreamSerializePrimitives(Writer1, Writer2, Test);
}
TEST(Serialization_FakeStream, SerializeCString)
{
    serialization::WriteStream Writer1(MaxMTUSize);
    serialization::FakeWriteStream Writer2(MaxMTUSize);

    UnitTest_CompareStreamSerializePrimitives(Writer1, Writer2, "TestString\0");
}
TEST(Serialization_FakeStream, SerializeEnum)
{
    serialization::WriteStream Writer1(MaxMTUSize);
    serialization::FakeWriteStream Writer2(MaxMTUSize);

    UnitTest_CompareStreamSerializePrimitives(Writer1, Writer2, EnumTest::B);
}