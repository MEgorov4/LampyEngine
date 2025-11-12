#pragma once

#include "MemoryMacros.h"
#include <cstddef>
#include <limits>
#include <new>
#include <utility>

namespace EngineCore::Foundation
{

/**
 * @brief STL-compatible allocator for resources using MemorySystem
 * Uses MemoryTag::Resource for all allocations
 */
template <typename T>
struct ResourceAllocator
{
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;

    template <typename U>
    struct rebind
    {
        using other = ResourceAllocator<U>;
    };

    ResourceAllocator() noexcept = default;
    
    template <class U>
    constexpr ResourceAllocator(const ResourceAllocator<U>&) noexcept
    {
    }

    [[nodiscard]] T* allocate(std::size_t n)
    {
        if (n == 0)
            return nullptr;
        
        // Use MemorySystem with MemoryTag::Resource
        // AllocateMemory already calls Profiler::Alloc
        void* p = AllocateMemory(n * sizeof(T), alignof(T), MemoryTag::Resource);
        
        if (p == nullptr)
        {
            throw std::bad_alloc();
        }
        
        return static_cast<T*>(p);
    }

    void deallocate(T* p, std::size_t n) noexcept
    {
        if (p == nullptr)
            return;
        
        // DeallocateMemory already calls Profiler::Free
        (void)n; // Size is not needed for deallocation
        DeallocateMemory(p, MemoryTag::Resource);
    }

    template <typename U, typename... Args>
    void construct(U* p, Args&&... args)
    {
        ::new (static_cast<void*>(p)) U(std::forward<Args>(args)...);
    }

    template <typename U>
    void destroy(U* p)
    {
        p->~U();
    }

    size_type max_size() const noexcept
    {
        return std::numeric_limits<size_type>::max() / sizeof(T);
    }
};

template <class T, class U>
bool operator==(const ResourceAllocator<T>&, const ResourceAllocator<U>&) noexcept
{
    return true;
}

template <class T, class U>
bool operator!=(const ResourceAllocator<T>&, const ResourceAllocator<U>&) noexcept
{
    return false;
}

} // namespace EngineCore::Foundation

