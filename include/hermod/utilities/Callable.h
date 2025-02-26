#pragma once

#include <cassert>
#include <memory>
#include <array>

template <typename>
class Callable;

template <typename Result, typename... Arguments>
class Callable<Result(Arguments...)>
{
    template <typename ReturnType, typename... Args>
    struct FunctorHolderBase
    {
        virtual ~FunctorHolderBase() {}
        virtual ReturnType operator()(Args&&...) = 0;
        virtual void copyInto(void*) const = 0;
        virtual FunctorHolderBase<Result, Arguments...>* clone() const = 0;
    };

    template <typename Functor, typename ReturnType, typename... Args>
    struct FunctorHolder final : FunctorHolderBase<Result, Arguments...>
    {
        FunctorHolder(Functor func) : f(func) {}

        ReturnType operator()(Args&&... args) override
        {
            return f(std::forward<Arguments>(args)...);
        }

        void copyInto(void* destination) const override
        {
            new (destination) FunctorHolder(f);
        }

        FunctorHolderBase<Result, Arguments...>* clone() const override
        {
            return new FunctorHolder(f);
        }

        Functor f;
    };

    FunctorHolderBase<Result, Arguments...>* FunctorHolderPtr = nullptr;

#if _HAS_CXX23
    alignas(char) std::byte Stack[32*sizeof(char)];
#else
    typename std::aligned_storage<32>::type Stack;
#endif

public:

    bool IsValid() const
    {
        return FunctorHolderPtr != nullptr;
    }

    operator bool() const
    {
        return IsValid();
    }

    template <typename Functor>
    Callable(Functor f)
    {
        if (sizeof(FunctorHolder<Functor, Result, Arguments...>) <= sizeof(Stack))
        {
            FunctorHolderPtr = (decltype (FunctorHolderPtr))std::addressof(Stack);
            new (FunctorHolderPtr) FunctorHolder<Functor, Result, Arguments...>(f);
        }
        else
        {
            FunctorHolderPtr = new FunctorHolder<Functor, Result, Arguments...>(f);
        }
    }

    Callable(const Callable& other)
    {
        if (other.FunctorHolderPtr != nullptr)
        {
            if (other.FunctorHolderPtr == (decltype (other.FunctorHolderPtr))std::addressof(other.Stack))
            {
                FunctorHolderPtr = (decltype (FunctorHolderPtr))std::addressof(Stack);
                other.FunctorHolderPtr->copyInto(FunctorHolderPtr);
            }
            else
            {
                FunctorHolderPtr = other.FunctorHolderPtr->clone();
            }
        }
    }

    Callable& operator= (Callable const& other)
    {
        if (FunctorHolderPtr != nullptr)
        {
            if (FunctorHolderPtr == (decltype (FunctorHolderPtr))std::addressof(Stack))
                FunctorHolderPtr->~FunctorHolderBase();
            else
                delete FunctorHolderPtr;

            FunctorHolderPtr = nullptr;
        }

        if (other.FunctorHolderPtr != nullptr)
        {
            if (other.FunctorHolderPtr == (decltype (other.FunctorHolderPtr))std::addressof(other.Stack))
            {
                FunctorHolderPtr = (decltype (FunctorHolderPtr))std::addressof(Stack);
                other.FunctorHolderPtr->copyInto(FunctorHolderPtr);
            }
            else
            {
                FunctorHolderPtr = other.FunctorHolderPtr->clone();
            }
        }

        return *this;
    }
    Callable() = default;

    ~Callable()
    {
        if (FunctorHolderPtr == (decltype (FunctorHolderPtr))std::addressof(Stack))
            FunctorHolderPtr->~FunctorHolderBase();
        else
            delete FunctorHolderPtr;
    }

    Result operator() (Arguments... args) const
    {
        assert(IsValid());
        return (*FunctorHolderPtr) (std::forward<Arguments>(args)...);
    }

};
