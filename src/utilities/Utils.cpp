#include <hermod/utilities/Utils.h>
#include <vector>
#include <string>
#include <sstream>

namespace utils
{

    std::vector<std::string> split(const std::string& s, char delim) {
        std::vector<std::string> result;
        std::stringstream ss(s);
        std::string item;

        while (getline(ss, item, delim)) {
            result.push_back(item);
        }

        return result;
    }


    bool sequence_greater_than(uint16_t s1, uint16_t s2)
    {
        return ((s1 > s2) && (s1 - s2 <= 32768)) ||
            ((s1 < s2) && (s2 - s1 > 32768));
    }

    uint16_t sequence_modulo(int s1, uint16_t MaxValue /*= uint16_t_MAX*/)
    {
        if (s1 < 0)
        {
            return MaxValue + 1 + s1;
        }

        return s1 % MaxValue;
    }

    // count the number of set bits (1s) in an unsigned integer
    uint32_t popcount(uint32_t x)
    {
#ifdef __GNUC__
        return __builtin_popcount(x);
#else // #ifdef __GNUC__
        const uint32_t a = x - ((x >> 1) & 0x55555555);
        const uint32_t b = (((a >> 2) & 0x33333333) + (a & 0x33333333));
        const uint32_t c = (((b >> 4) + b) & 0x0f0f0f0f);
        const uint32_t d = c + (c >> 8);
        const uint32_t e = d + (d >> 16);
        const uint32_t result = e & 0x0000003f;
        return result;
#endif // #ifdef __GNUC__
    }

    uint32_t log2(uint32_t x)
    {
        const uint32_t a = x | (x >> 1);
        const uint32_t b = a | (a >> 2);
        const uint32_t c = b | (b >> 4);
        const uint32_t d = c | (c >> 8);
        const uint32_t e = d | (d >> 16);
        const uint32_t f = e >> 1;
        return popcount(f);
    }

    int bits_required(uint32_t min, uint32_t max)
    {
        return (min == max) ? 0 : log2(max - min) + 1;
    }
}