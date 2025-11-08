#pragma once
#include "Profiler.h"
#include <memory>

namespace EngineCore::Foundation
{
template <typename T> struct ProfileAllocator
{
    using value_type = T;

    ProfileAllocator() noexcept = default;
    template <class U> constexpr ProfileAllocator(const ProfileAllocator<U> &) noexcept
    {
    }

    [[nodiscard]] T *allocate(std::size_t n)
    {
        if (n == 0)
            return nullptr;
        void *p = ::operator new(n * sizeof(T));
        Profiler::Alloc(p, n * sizeof(T), typeid(T).name());
        return static_cast<T *>(p);
    }

    void deallocate(T *p, std::size_t n) noexcept
    {
        Profiler::Free(p, typeid(T).name());
        ::operator delete(p);
    }
};

template <class T, class U> bool operator==(const ProfileAllocator<T> &, const ProfileAllocator<U> &)
{
    return true;
}

template <class T, class U> bool operator!=(const ProfileAllocator<T> &, const ProfileAllocator<U> &)
{
    return false;
}
} // namespace EngineCore::Foundation
