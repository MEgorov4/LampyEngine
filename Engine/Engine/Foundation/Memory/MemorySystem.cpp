#include "MemorySystem.h"
#include "../Log/Log.h"
#include "../Assert/Assert.h"

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif

#include <cstdlib>
#include <algorithm>

namespace EngineCore::Foundation
{

std::vector<MemorySystem::AllocatorEntry> MemorySystem::s_allocators;
std::mutex MemorySystem::s_mutex;
MemorySystem::Statistics MemorySystem::s_statistics;
bool MemorySystem::s_initialized = false;
LinearAllocator* MemorySystem::s_frameAllocator = nullptr;
FreeListAllocator* MemorySystem::s_persistentAllocator = nullptr;

void MemorySystem::startup(size_t frameAllocatorSize, size_t persistentAllocatorSize)
{
    std::lock_guard<std::mutex> lock(s_mutex);

    if (s_initialized)
    {
        LT_LOG(LogVerbosity::Warning, "MemorySystem", "Already initialized");
        return;
    }

    LT_LOG(LogVerbosity::Info, "MemorySystem", "Initializing memory system...");

    // Allocate memory for frame allocator
    auto frameMemory = std::make_unique<uint8_t[]>(frameAllocatorSize);
    void* frameMemoryPtr = frameMemory.get();
    auto frameAlloc = std::make_unique<LinearAllocator>(frameMemoryPtr, frameAllocatorSize, MemoryTag::Temp);
    s_frameAllocator = frameAlloc.get();

    s_allocators.push_back({
        std::move(frameAlloc),
        std::move(frameMemory),
        MemoryTag::Temp
    });

    // Allocate memory for persistent allocator
    auto persistentMemory = std::make_unique<uint8_t[]>(persistentAllocatorSize);
    void* persistentMemoryPtr = persistentMemory.get();
    auto persistentAlloc = std::make_unique<FreeListAllocator>(persistentMemoryPtr, persistentAllocatorSize, MemoryTag::Unknown);
    s_persistentAllocator = persistentAlloc.get();

    s_allocators.push_back({
        std::move(persistentAlloc),
        std::move(persistentMemory),
        MemoryTag::Unknown
    });

    s_statistics = Statistics{};
    s_initialized = true;

    LT_LOG(LogVerbosity::Info, "MemorySystem", 
        std::format("Memory system initialized - Frame: {}MB, Persistent: {}MB",
            frameAllocatorSize / (1024 * 1024),
            persistentAllocatorSize / (1024 * 1024)));
}

void MemorySystem::shutdown() noexcept
{
    std::lock_guard<std::mutex> lock(s_mutex);

    if (!s_initialized)
        return;

    LT_LOG(LogVerbosity::Info, "MemorySystem", "Shutting down memory system...");

    // Print final statistics
    auto stats = getStatistics();
    LT_LOG(LogVerbosity::Info, "MemorySystem",
        std::format("Final stats - Allocated: {} bytes, Peak: {} bytes, Allocs: {}, Deallocs: {}",
            stats.allocatedBytes, stats.peakBytes, stats.allocCount, stats.deallocCount));

    if (stats.allocatedBytes != 0)
    {
        LT_LOG(LogVerbosity::Error, "MemorySystem",
               std::format("Memory leak detected: {} bytes still allocated", stats.allocatedBytes));

        for (uint8_t tag = 0; tag < static_cast<uint8_t>(MemoryTag::Count); ++tag)
        {
            auto tagStats = getStatistics(static_cast<MemoryTag>(tag));
            if (tagStats.allocatedBytes == 0)
                continue;

            LT_LOG(LogVerbosity::Warning, "MemorySystem",
                   std::format("Tag [{}] still allocated {} bytes",
                               getMemoryTagName(static_cast<MemoryTag>(tag)),
                               tagStats.allocatedBytes));
        }
    }

    s_allocators.clear();
    s_frameAllocator = nullptr;
    s_persistentAllocator = nullptr;
    s_initialized = false;
}

IAllocator& MemorySystem::getFrameAllocator() noexcept
{
    std::lock_guard<std::mutex> lock(s_mutex);
    LT_ASSERT(s_initialized && s_frameAllocator != nullptr);
    return *s_frameAllocator;
}

IAllocator& MemorySystem::getPersistentAllocator() noexcept
{
    std::lock_guard<std::mutex> lock(s_mutex);
    LT_ASSERT(s_initialized && s_persistentAllocator != nullptr);
    return *s_persistentAllocator;
}

LinearAllocator* MemorySystem::createLinearAllocator(size_t size, MemoryTag tag)
{
    auto memory = std::make_unique<uint8_t[]>(size);
    void* memoryPtr = memory.get();
    auto allocator = std::make_unique<LinearAllocator>(memoryPtr, size, tag);
    LinearAllocator* allocatorPtr = allocator.get();
    
    // Store memory ownership in s_allocators
    std::lock_guard<std::mutex> lock(s_mutex);
    s_allocators.push_back({
        std::move(allocator),
        std::move(memory),
        tag
    });

    // Return raw pointer (memory is owned by s_allocators)
    return allocatorPtr;
}

StackAllocator* MemorySystem::createStackAllocator(size_t size, MemoryTag tag)
{
    auto memory = std::make_unique<uint8_t[]>(size);
    void* memoryPtr = memory.get();
    auto allocator = std::make_unique<StackAllocator>(memoryPtr, size, tag);
    StackAllocator* allocatorPtr = allocator.get();
    
    std::lock_guard<std::mutex> lock(s_mutex);
    s_allocators.push_back({
        std::move(allocator),
        std::move(memory),
        tag
    });

    return allocatorPtr;
}

PoolAllocator* MemorySystem::createPoolAllocator(size_t size, size_t blockSize, MemoryTag tag)
{
    auto memory = std::make_unique<uint8_t[]>(size);
    void* memoryPtr = memory.get();
    auto allocator = std::make_unique<PoolAllocator>(memoryPtr, size, blockSize, tag);
    PoolAllocator* allocatorPtr = allocator.get();
    
    std::lock_guard<std::mutex> lock(s_mutex);
    s_allocators.push_back({
        std::move(allocator),
        std::move(memory),
        tag
    });

    return allocatorPtr;
}

FreeListAllocator* MemorySystem::createFreeListAllocator(size_t size, MemoryTag tag)
{
    auto memory = std::make_unique<uint8_t[]>(size);
    void* memoryPtr = memory.get();
    auto allocator = std::make_unique<FreeListAllocator>(memoryPtr, size, tag);
    FreeListAllocator* allocatorPtr = allocator.get();
    
    std::lock_guard<std::mutex> lock(s_mutex);
    s_allocators.push_back({
        std::move(allocator),
        std::move(memory),
        tag
    });

    return allocatorPtr;
}

void MemorySystem::resetFrameAllocator() noexcept
{
    if (s_frameAllocator)
    {
        size_t usedBefore = s_frameAllocator->getUsed();
        s_frameAllocator->reset();
        
        // Update statistics
        {
            std::lock_guard<std::mutex> lock(s_mutex);
            s_statistics.deallocCount++;
            if (s_statistics.allocatedBytes >= usedBefore)
            {
                s_statistics.allocatedBytes -= usedBefore;
            }
        }
        
#ifdef TRACY_ENABLE
        TracyMessage("FrameAllocator reset", 19);
#endif
    }
}

MemorySystem::Statistics MemorySystem::getStatistics() noexcept
{
    return s_statistics;
}

MemorySystem::Statistics MemorySystem::getStatistics(MemoryTag tag) noexcept
{
    Statistics stats{};
    
    std::lock_guard<std::mutex> lock(s_mutex);
    for (const auto& entry : s_allocators)
    {
        if (entry.tag == tag)
        {
            stats.allocatedBytes += entry.allocator->getUsed();
            stats.peakBytes = std::max(stats.peakBytes, entry.allocator->getUsed());
        }
    }

    return stats;
}

const char* MemorySystem::getMemoryTagName(MemoryTag tag) noexcept
{
    switch (tag)
    {
    case MemoryTag::Unknown: return "Unknown";
    case MemoryTag::Render: return "Render";
    case MemoryTag::Physics: return "Physics";
    case MemoryTag::Audio: return "Audio";
    case MemoryTag::UI: return "UI";
    case MemoryTag::Temp: return "Temp";
    case MemoryTag::ECS: return "ECS";
    case MemoryTag::Resource: return "Resource";
    case MemoryTag::Script: return "Script";
    default: return "Invalid";
    }
}

void* MemorySystem::allocateSystemMemory(size_t size)
{
    // This function is no longer used - we use std::make_unique instead
    // Keeping for potential future use
    (void)size;
    return nullptr;
}

void MemorySystem::deallocateSystemMemory(void* ptr, size_t size) noexcept
{
    // This function is no longer used
    (void)ptr;
    (void)size;
}

} // namespace EngineCore::Foundation

