#pragma once

#include <hermod/platform/Platform.h>

#include <vector>
#include <string>
#include <cassert>
#include <bit>
#include <bitset>


#define PROTO_SERIALIZE_CHECKS              1
//#define PROTO_DEBUG_PACKET_LEAKS            0
//#define PROTO_PACKET_AGGREGATION            1

namespace utils
{
    std::vector<std::string> split(const std::string& s, char delim);

    bool sequence_greater_than(uint16_t s1, uint16_t s2);

    uint16_t sequence_modulo(int s1, uint16_t MaxValue);

    /*
    template<typename E>
    constexpr auto to_integral(E e) -> typename std::underlying_type<E>::type
    {
        return static_cast<typename std::underlying_type<E>::type>(e);
    }*/

    template <uint32_t x> struct PopCount
    {
        enum {
            a = x - ((x >> 1) & 0x55555555),
            b = (((a >> 2) & 0x33333333) + (a & 0x33333333)),
            c = (((b >> 4) + b) & 0x0f0f0f0f),
            d = c + (c >> 8),
            e = d + (d >> 16),

            result = e & 0x0000003f
        };
    };

    template <uint32_t x> struct Log2
    {
        enum{
            a = x | (x >> 1),
            b = a | (a >> 2),
            c = b | (b >> 4),
            d = c | (c >> 8),
            e = d | (d >> 16),
            f = e >> 1,

            result = PopCount<f>::result
        };
    };

    template <int64_t min, int64_t max> struct BitsRequired
    {
        static const uint32_t result = (min == max) ? 0 : (Log2<uint32_t(max - min)>::result + 1);
    };

#define BITS_REQUIRED( min, max ) BitsRequired<min,max>::result

    // count the number of set bits (1s) in an unsigned integer
    uint32_t popcount(uint32_t x);

    uint32_t log2(uint32_t x);

    HERMOD_API int bits_required(uint32_t min, uint32_t max);


    template < std::size_t InvalidValue = ULLONG_MAX >
    class IIntrusiveElement
    {
        uint8_t	Index = InvalidValue;

    public:

        const bool HasIndex() const
        {
            return Index != InvalidValue;
        }

        void Own(std::size_t IndexInParent)
        {
            Index = static_cast<uint8_t>(IndexInParent);;
        }
        std::size_t Release()
        {
            std::size_t IndexInParent = static_cast<std::size_t>(Index);
            Index = InvalidValue;
            return IndexInParent;
        }

        const std::size_t GetIndex() const
        {
            assert(Index != InvalidValue);
            return Index;
        }
    };  

    template <std::size_t Size, std::derived_from<IIntrusiveElement<Size>> T >
    class FixedIntrusiveArray
    {
        //static_assert(Size <= (sizeof(unsigned long long) * 8), "Max size is ULLONG_MAX * 8");
        using ValueType = T; // IIntrusiveElement<Size>
        using Pointer = ValueType*;
        using Reference = ValueType&;

        Pointer Values[Size];
        std::bitset<Size> ValidIndexes;
        std::size_t NextFreeIndex;
        std::size_t Num;

        enum Computation
        {
            ComputeFreeIndex = 1 << 1,
            ComputeNum = 1 << 2,
            Both = ComputeFreeIndex | ComputeNum
        };
        void ComputeNums(Computation InComputeOpt)
        {
            const bool CompFreeIndex = (InComputeOpt & ComputeFreeIndex) != 0;
            const bool CompNum = (InComputeOpt & ComputeNum) != 0;
            if (CompFreeIndex)
            {
                NextFreeIndex = 0;
            }
            int CurRZeros = 0;
            constexpr int SizeBitOfWord = (sizeof(uint64_t) * CHAR_BIT);
            const int End = 1 + (Size / SizeBitOfWord);
            for (int Idx = 0; Idx < End; ++Idx)
            {
                uint64_t CurWord = ValidIndexes._Getword(Idx);
                if (CompFreeIndex && NextFreeIndex == (SizeBitOfWord * Idx))
                {
                    NextFreeIndex += std::countr_one(CurWord);
                }

                if (CompNum)
                {
                    int CurRZerosInWord = std::countl_zero(CurWord);
                    if (CurRZerosInWord != SizeBitOfWord)
                    {
                        CurRZeros = 0;
                    }

                    CurRZeros += CurRZerosInWord;
                }
            }
            if (CompNum)
            {
                Num = (1+Size) - CurRZeros;
            }
        }

    public:
        
        FixedIntrusiveArray()
            : NextFreeIndex(0)
            , Num(0)
        {
            ValidIndexes.reset();
            memset(Values, 0, sizeof(Pointer) * Size);
        }

        void Add(Reference InValue)
        {
            Values[NextFreeIndex] = &InValue;
            ValidIndexes.set(NextFreeIndex);
            InValue.Own(NextFreeIndex);

            ComputeNums(Both);
        }

        void Remove(const Reference InValue)
        {
            NextFreeIndex = InValue.Release();
            memset(*(Values + NextFreeIndex), 0, sizeof(Pointer) * Size);
            ValidIndexes.set(NextFreeIndex, false);

            ComputeNums(ComputeNum);
        }

        void Count() const
        {
            return Num;
        }


        class iterator {
            Pointer* Current;
        public:
            // iterator traits
            using difference_type = std::size_t;
            using value_type = ValueType;
            using pointer = Pointer*;
            using reference = Reference;
            using iterator_category = std::forward_iterator_tag;

            iterator(pointer StartFrom) : Current(StartFrom) {}
            iterator& operator++() { Current++; return *this; }
            iterator operator++(int) { iterator retval = *this; ++(*this); return retval; }
            bool operator==(iterator other) const { return Current == other.Current; }
            bool operator!=(iterator other) const { return !(*this == other); }
            Pointer operator*() { return *Current; }
        };
        class const_iterator {
            const Pointer* Current;
        public:
            // const_iterator traits
            using difference_type = std::size_t;
            using value_type = ValueType;
            using pointer = const Pointer*;
            using reference = const Reference;
            using iterator_category = std::forward_iterator_tag;

            const_iterator(const pointer StartFrom) : Current(StartFrom) {}
            const_iterator& operator++() { Current++; return *this; }
            const_iterator operator++(int) { const_iterator retval = *this; ++(*this); return retval; }
            bool operator==(const_iterator other) const { return Current == other.Current; }
            bool operator!=(const_iterator other) const { return !(*this == other); }
            Pointer operator*() { return *Current; }
        };

        iterator begin() { return Values; }
        iterator end() { return Values+ Num; }
        const_iterator cbegin() { return Values; }
        const_iterator cend() { return Values + Size; }
    };
}