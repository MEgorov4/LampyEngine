#ifdef ENABLE_GLOBAL_MEMORY_TRACKING

#include "GlobalMemoryTracking.h"
#include "Profiler.h"
#include <cstdlib>
#include <new>

#ifdef _WIN32
#include <malloc.h>
#else
#include <stdlib.h>
#endif

using namespace EngineCore::Foundation;

// Standard new operators
void* operator new(std::size_t size)
{
    void* ptr = std::malloc(size);
    if (!ptr)
        throw std::bad_alloc();
    
    Profiler::Alloc(ptr, size, "global_new");
    return ptr;
}

void* operator new[](std::size_t size)
{
    void* ptr = std::malloc(size);
    if (!ptr)
        throw std::bad_alloc();
    
    Profiler::Alloc(ptr, size, "global_new[]");
    return ptr;
}

void* operator new(std::size_t size, const std::nothrow_t&) noexcept
{
    void* ptr = std::malloc(size);
    if (ptr)
        Profiler::Alloc(ptr, size, "global_new_nothrow");
    return ptr;
}

void* operator new[](std::size_t size, const std::nothrow_t&) noexcept
{
    void* ptr = std::malloc(size);
    if (ptr)
        Profiler::Alloc(ptr, size, "global_new[]_nothrow");
    return ptr;
}

// Standard delete operators
void operator delete(void* ptr) noexcept
{
    if (ptr)
    {
        Profiler::Free(ptr, "global_delete");
        std::free(ptr);
    }
}

void operator delete[](void* ptr) noexcept
{
    if (ptr)
    {
        Profiler::Free(ptr, "global_delete[]");
        std::free(ptr);
    }
}

void operator delete(void* ptr, const std::nothrow_t&) noexcept
{
    if (ptr)
    {
        Profiler::Free(ptr, "global_delete_nothrow");
        std::free(ptr);
    }
}

void operator delete[](void* ptr, const std::nothrow_t&) noexcept
{
    if (ptr)
    {
        Profiler::Free(ptr, "global_delete[]_nothrow");
        std::free(ptr);
    }
}

// C++17 aligned new/delete operators
#if __cplusplus >= 201703L

// Portable aligned allocation
static void* aligned_allocate(std::size_t size, std::size_t alignment)
{
#ifdef _WIN32
    return _aligned_malloc(size, alignment);
#else
    void* ptr = nullptr;
    if (posix_memalign(&ptr, alignment, size) != 0)
        return nullptr;
    return ptr;
#endif
}

static void aligned_deallocate(void* ptr)
{
#ifdef _WIN32
    _aligned_free(ptr);
#else
    std::free(ptr);
#endif
}

void* operator new(std::size_t size, std::align_val_t alignment)
{
    void* ptr = aligned_allocate(size, static_cast<std::size_t>(alignment));
    if (!ptr)
        throw std::bad_alloc();
    
    Profiler::Alloc(ptr, size, "global_new_aligned");
    return ptr;
}

void* operator new[](std::size_t size, std::align_val_t alignment)
{
    void* ptr = aligned_allocate(size, static_cast<std::size_t>(alignment));
    if (!ptr)
        throw std::bad_alloc();
    
    Profiler::Alloc(ptr, size, "global_new[]_aligned");
    return ptr;
}

void* operator new(std::size_t size, std::align_val_t alignment, const std::nothrow_t&) noexcept
{
    void* ptr = aligned_allocate(size, static_cast<std::size_t>(alignment));
    if (ptr)
        Profiler::Alloc(ptr, size, "global_new_aligned_nothrow");
    return ptr;
}

void* operator new[](std::size_t size, std::align_val_t alignment, const std::nothrow_t&) noexcept
{
    void* ptr = aligned_allocate(size, static_cast<std::size_t>(alignment));
    if (ptr)
        Profiler::Alloc(ptr, size, "global_new[]_aligned_nothrow");
    return ptr;
}

void operator delete(void* ptr, std::align_val_t alignment) noexcept
{
    if (ptr)
    {
        Profiler::Free(ptr, "global_delete_aligned");
        aligned_deallocate(ptr);
    }
}

void operator delete[](void* ptr, std::align_val_t alignment) noexcept
{
    if (ptr)
    {
        Profiler::Free(ptr, "global_delete[]_aligned");
        aligned_deallocate(ptr);
    }
}

void operator delete(void* ptr, std::align_val_t alignment, const std::nothrow_t&) noexcept
{
    if (ptr)
    {
        Profiler::Free(ptr, "global_delete_aligned_nothrow");
        aligned_deallocate(ptr);
    }
}

void operator delete[](void* ptr, std::align_val_t alignment, const std::nothrow_t&) noexcept
{
    if (ptr)
    {
        Profiler::Free(ptr, "global_delete[]_aligned_nothrow");
        aligned_deallocate(ptr);
    }
}

#endif // __cplusplus >= 201703L

#endif // ENABLE_GLOBAL_MEMORY_TRACKING

