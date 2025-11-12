#pragma once

#include "IAllocator.h"
#include "LinearAllocator.h"
#include "StackAllocator.h"
#include "PoolAllocator.h"
#include "FreeListAllocator.h"
#include <memory>
#include <vector>
#include <mutex>

namespace EngineCore::Foundation
{

/**
 * @brief Central memory management system for the engine
 * Manages different allocators for different use cases
 */
class MemorySystem
{
public:
    struct Statistics
    {
        size_t allocatedBytes = 0;
        size_t peakBytes = 0;
        size_t allocCount = 0;
        size_t deallocCount = 0;
    };

    static void startup(size_t frameAllocatorSize = 2 * 1024 * 1024, // 2MB default
                        size_t persistentAllocatorSize = 64 * 1024 * 1024); // 64MB default

    static void shutdown() noexcept;

    /**
     * @brief Get frame allocator (reset each frame)
     */
    [[nodiscard]] static IAllocator& getFrameAllocator() noexcept;

    /**
     * @brief Get persistent allocator (long-lived allocations)
     */
    [[nodiscard]] static IAllocator& getPersistentAllocator() noexcept;

    /**
     * @brief Create a custom linear allocator
     * @return Raw pointer to the allocator (owned by MemorySystem)
     */
    [[nodiscard]] static LinearAllocator* createLinearAllocator(
        size_t size, MemoryTag tag = MemoryTag::Temp);

    /**
     * @brief Create a custom stack allocator
     * @return Raw pointer to the allocator (owned by MemorySystem)
     */
    [[nodiscard]] static StackAllocator* createStackAllocator(
        size_t size, MemoryTag tag = MemoryTag::Temp);

    /**
     * @brief Create a custom pool allocator
     * @return Raw pointer to the allocator (owned by MemorySystem)
     */
    [[nodiscard]] static PoolAllocator* createPoolAllocator(
        size_t size, size_t blockSize, MemoryTag tag = MemoryTag::ECS);

    /**
     * @brief Create a custom free list allocator
     * @return Raw pointer to the allocator (owned by MemorySystem)
     */
    [[nodiscard]] static FreeListAllocator* createFreeListAllocator(
        size_t size, MemoryTag tag = MemoryTag::Unknown);

    /**
     * @brief Reset frame allocator (call at end of each frame)
     */
    static void resetFrameAllocator() noexcept;

    /**
     * @brief Get memory statistics
     */
    [[nodiscard]] static Statistics getStatistics() noexcept;

    /**
     * @brief Get statistics for a specific tag
     */
    [[nodiscard]] static Statistics getStatistics(MemoryTag tag) noexcept;

    /**
     * @brief Get memory tag name as string
     */
    [[nodiscard]] static const char* getMemoryTagName(MemoryTag tag) noexcept;

private:
    struct AllocatorEntry
    {
        std::unique_ptr<IAllocator> allocator;
        std::unique_ptr<uint8_t[]> memory;
        MemoryTag tag;
    };

    static void* allocateSystemMemory(size_t size);
    static void deallocateSystemMemory(void* ptr, size_t size) noexcept;

    static std::vector<AllocatorEntry> s_allocators;
    static std::mutex s_mutex;
    static Statistics s_statistics;
    static bool s_initialized;

    static LinearAllocator* s_frameAllocator;
    static FreeListAllocator* s_persistentAllocator;
    
    // Allow MemoryMacros to access private members
    friend void* AllocateMemory(size_t size, size_t alignment, MemoryTag tag);
    friend void DeallocateMemory(void* ptr, MemoryTag tag);
};

} // namespace EngineCore::Foundation

