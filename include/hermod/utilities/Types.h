#pragma once

#include <stdio.h>
#include <stdint.h>
#include <limits>
#include <memory>

#include <hermod/utilities/Utils.h>
#include <hermod/utilities/TypeTraits.h>

typedef double TimeMs;

const unsigned int DefaultProtocolId = 666;
const int MaxMTUSize = 1472;
const int MaxFragmentSize = 1024;
const int MaxStreamSize = 1024 * 64;
const int MaxNetObjectQueueSize = 256;
const int ProtocolHeaderSizeLessId = 12; // or 18?
const int ProtocolHeaderSize = 16; // or 18?
const int FragmentHeaderSize = 6;

namespace serialization
{
	typedef std::shared_ptr<struct NetObjectData> NetObjectDataPtr;
}

namespace proto
{
	typedef class NetObjectQueue NetObjectQueue256;
}

enum EReliability
{
	Unreliable,
	Reliable
};

using NetObjectId = uint64_t;
static constexpr NetObjectId InvalidNetObjectId = std::numeric_limits<uint64_t>::max();
#define PTR_TO_ID_MASK 0xfffffffffffffffull
#define PTR_TO_ID(ObjectPtr) (((NetObjectId)ObjectPtr) & PTR_TO_ID_MASK)
#define ID_TO_PTR(Type, Id) ((Type)Id)

// compile time FNV-1a
constexpr uint32_t Hash32_CT(const char* str, size_t n, uint32_t basis = UINT32_C(2166136261)) {
	return n == 0 ? basis : Hash32_CT(str + 1, n - 1, (basis ^ str[0]) * UINT32_C(16777619));
}
/*
template< uint32_t id >
uint32_t typeid_helper() {
	return id;
}*/

template< uint32_t id >
struct typeid_helper {
	static const uint32_t value = id;
};

#define TYPEID( T ) typeid_helper< Hash32_CT( #T, sizeof( #T ) - 1 ) >
#define TYPEID_VAL( T ) TYPEID(T)::value

#if _HAS_CXX20
template <typename T> concept THasClassIdFunc = requires(T t) { 
	{ t.GetClassId() } -> std::same_as<uint32_t>;
};


namespace type {
	template <THasClassIdFunc T1, THasClassIdFunc T2>
	static bool is_a(const T2 &NetObject) {
		return T1::NetObjectId::value == NetObject.GetClassId();
	}

	template <THasClassIdFunc T1>
	static bool is_a(const uint32_t &NetObjectId) {
		return T1::NetObjectId::value == NetObjectId;
	}
}

#else
GENERATE_MEMBER_FUNCTION_CHECK(GetClassId, uint32_t, const)

namespace type {
	/*template <typename T1, typename T2>
	static bool is_a(const T2 &NetObject)
	{ }*/

	template <typename T1, typename T2>
	static typename enable_if<has_member_function_GetClassId<T1>::Value && has_member_function_GetClassId<T2>::Value, bool>::Type is_a(const T2 &NetObject) {
		return T1::NetObjectId::value == NetObject.GetClassId();
	}

	template <typename T1>
	static typename enable_if<has_member_function_GetClassId<T1>::Value, bool>::Type is_a(const uint32_t &NetObjectId) {
		return T1::NetObjectId::value == NetObjectId;
	}
} //namespace type
#endif


#define INTERNAL_NETCLASS_ID(ClassName)				\
public:												\
	virtual uint32_t GetClassId() const	= 0;		\
private:


#define CLASS_ID(ClassName)							\
public:												\
	using NetObjectId = typeid_helper< Hash32_CT( #ClassName, sizeof( #ClassName ) - 1 ) >;			\
	virtual uint32_t GetClassId() const				\
	{												\
		return NetObjectId::value;					\
	}												\
private:
