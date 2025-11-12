#pragma once

#include "MemorySystem.h"
#include "../Profiler/Profiler.h"
#include <new>
#include <memory>

namespace EngineCore::Foundation
{

// Forward declarations
void* AllocateMemory(size_t size, size_t alignment, MemoryTag tag);
void DeallocateMemory(void* ptr, MemoryTag tag);
const char* GetMemoryTagName(MemoryTag tag) noexcept;

} // namespace EngineCore::Foundation

// Override global new/delete operators
#ifdef LT_USE_CUSTOM_ALLOCATORS

void* operator new(size_t size)
{
    return EngineCore::Foundation::AllocateMemory(size, alignof(std::max_align_t), EngineCore::Foundation::MemoryTag::Unknown);
}

void* operator new(size_t size, std::align_val_t alignment)
{
    return EngineCore::Foundation::AllocateMemory(size, static_cast<size_t>(alignment), EngineCore::Foundation::MemoryTag::Unknown);
}

void operator delete(void* ptr) noexcept
{
    EngineCore::Foundation::DeallocateMemory(ptr, EngineCore::Foundation::MemoryTag::Unknown);
}

void operator delete(void* ptr, std::align_val_t) noexcept
{
    EngineCore::Foundation::DeallocateMemory(ptr, EngineCore::Foundation::MemoryTag::Unknown);
}

void* operator new[](size_t size)
{
    return EngineCore::Foundation::AllocateMemory(size, alignof(std::max_align_t), EngineCore::Foundation::MemoryTag::Unknown);
}

void operator delete[](void* ptr) noexcept
{
    EngineCore::Foundation::DeallocateMemory(ptr, EngineCore::Foundation::MemoryTag::Unknown);
}

#endif // LT_USE_CUSTOM_ALLOCATORS

// Convenience macros for tagged allocation
#define LT_NEW(type, tag) \
    new(EngineCore::Foundation::AllocateMemory(sizeof(type), alignof(type), tag)) type

#define LT_NEW_ARRAY(type, count, tag) \
    static_cast<type*>(EngineCore::Foundation::AllocateMemory(sizeof(type) * (count), alignof(type), tag))

#define LT_DELETE(ptr, tag) \
    do { \
        if (ptr) { \
            using Type = std::remove_reference_t<decltype(*ptr)>; \
            ptr->~Type(); \
            EngineCore::Foundation::DeallocateMemory(ptr, tag); \
            ptr = nullptr; \
        } \
    } while(0)

#define LT_DELETE_ARRAY(ptr, count, tag) \
    do { \
        if (ptr) { \
            using Type = std::remove_reference_t<decltype(*ptr)>; \
            for (size_t i = 0; i < (count); ++i) { \
                ptr[i].~Type(); \
            } \
            EngineCore::Foundation::DeallocateMemory(ptr, tag); \
            ptr = nullptr; \
        } \
    } while(0)

