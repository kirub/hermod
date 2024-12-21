#pragma once
#include <cstdint>

namespace hash
{
    uint32_t crc32(const uint8_t* buffer, size_t length, uint32_t crc32);
    uint32_t data(const uint8_t* data, uint32_t length, uint32_t hash);
    uint32_t string(const char string[], uint32_t hash);
}