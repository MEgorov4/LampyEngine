#include "MemoryMacros.h"
#include "MemorySystem.h"
#include "../Profiler/Profiler.h"
#include <algorithm>

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif

namespace EngineCore::Foundation
{

void* AllocateMemory(size_t size, size_t alignment, MemoryTag tag)
{
    if (size == 0)
        return nullptr;

    IAllocator* allocator = nullptr;

    // Choose allocator based on tag
    switch (tag)
    {
    case MemoryTag::Temp:
        allocator = &MemorySystem::getFrameAllocator();
        break;
    default:
        allocator = &MemorySystem::getPersistentAllocator();
        break;
    }

    void* ptr = allocator->allocate(size, alignment);
    
    if (ptr == nullptr)
    {
        // Fallback to persistent allocator if frame allocator is full
        if (tag == MemoryTag::Temp)
        {
            allocator = &MemorySystem::getPersistentAllocator();
            ptr = allocator->allocate(size, alignment);
        }
    }

#ifdef TRACY_ENABLE
    if (ptr)
    {
        TracyAlloc(ptr, size);
    }
#endif

    Profiler::Alloc(ptr, size, GetMemoryTagName(tag));

    // Update statistics
    {
        std::lock_guard<std::mutex> lock(MemorySystem::s_mutex);
        MemorySystem::s_statistics.allocCount++;
        MemorySystem::s_statistics.allocatedBytes += size;
        MemorySystem::s_statistics.peakBytes = std::max(
            MemorySystem::s_statistics.peakBytes,
            MemorySystem::s_statistics.allocatedBytes);
    }

    return ptr;
}

void DeallocateMemory(void* ptr, MemoryTag tag)
{
    if (ptr == nullptr)
        return;

#ifdef TRACY_ENABLE
    TracyFree(ptr);
#endif

    Profiler::Free(ptr, GetMemoryTagName(tag));

    // Try frame allocator first (though it doesn't support individual deallocation)
    if (MemorySystem::getFrameAllocator().owns(ptr))
    {
        // Frame allocator doesn't support individual deallocation
        // Memory will be freed on reset
        // Update statistics
        {
            std::lock_guard<std::mutex> lock(MemorySystem::s_mutex);
            MemorySystem::s_statistics.deallocCount++;
            // Can't accurately track allocatedBytes for frame allocator
        }
        return;
    }

    // Use persistent allocator
    if (MemorySystem::getPersistentAllocator().owns(ptr))
    {
        size_t deallocatedSize = MemorySystem::getPersistentAllocator().getUsed();
        MemorySystem::getPersistentAllocator().deallocate(ptr);
        size_t newUsed = MemorySystem::getPersistentAllocator().getUsed();
        
        // Update statistics
        {
            std::lock_guard<std::mutex> lock(MemorySystem::s_mutex);
            MemorySystem::s_statistics.deallocCount++;
            // Estimate deallocated size (not perfect, but better than nothing)
            if (deallocatedSize > newUsed)
            {
                MemorySystem::s_statistics.allocatedBytes -= (deallocatedSize - newUsed);
            }
        }
    }
}

const char* GetMemoryTagName(MemoryTag tag) noexcept
{
    return MemorySystem::getMemoryTagName(tag);
}

} // namespace EngineCore::Foundation

