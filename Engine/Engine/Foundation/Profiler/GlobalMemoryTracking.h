#pragma once

// Global memory tracking for leak detection
// Overrides global new/delete operators to track all allocations via Profiler

#ifdef ENABLE_GLOBAL_MEMORY_TRACKING

#include <cstddef>
#include <new>

// Override global new/delete operators to track all memory allocations
// This allows Tracy to visualize all memory allocations, even those not using ProfileAllocator

void* operator new(std::size_t size);
void* operator new[](std::size_t size);
void* operator new(std::size_t size, const std::nothrow_t&) noexcept;
void* operator new[](std::size_t size, const std::nothrow_t&) noexcept;

void operator delete(void* ptr) noexcept;
void operator delete[](void* ptr) noexcept;
void operator delete(void* ptr, const std::nothrow_t&) noexcept;
void operator delete[](void* ptr, const std::nothrow_t&) noexcept;

// C++17 aligned new/delete
#if __cplusplus >= 201703L
void* operator new(std::size_t size, std::align_val_t alignment);
void* operator new[](std::size_t size, std::align_val_t alignment);
void* operator new(std::size_t size, std::align_val_t alignment, const std::nothrow_t&) noexcept;
void* operator new[](std::size_t size, std::align_val_t alignment, const std::nothrow_t&) noexcept;

void operator delete(void* ptr, std::align_val_t alignment) noexcept;
void operator delete[](void* ptr, std::align_val_t alignment) noexcept;
void operator delete(void* ptr, std::align_val_t alignment, const std::nothrow_t&) noexcept;
void operator delete[](void* ptr, std::align_val_t alignment, const std::nothrow_t&) noexcept;
#endif

#endif // ENABLE_GLOBAL_MEMORY_TRACKING

