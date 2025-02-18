#pragma once

#include <cstdint>


#define PROPERTY(Value) *this,Value

static const uint8_t MaxPropertyPerObject = UINT8_MAX;
static const uint8_t InvalidPropertyIndex = MaxPropertyPerObject;