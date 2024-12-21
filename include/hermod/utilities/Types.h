#pragma once

#include <stdio.h>
#include <stdint.h>

typedef double TimeMs;

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

#define TYPEID( T ) typeid_helper< Hash32_CT( #T, sizeof( #T ) - 1 ) >::value
#define DECLARE_ID( T ) typename typeid_helper<#T> Id



#define CLASS_ID(ClassName)							\
public:												\
	static uint32_t StaticClassId()					\
	{												\
		return TYPEID(ClassName);					\
	}												\
	virtual uint32_t GetClassId() const				\
	{												\
		return ClassName::StaticClassId();			\
	}												\
private: