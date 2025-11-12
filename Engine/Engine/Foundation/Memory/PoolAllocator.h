#pragma once

#include "IAllocator.h"
#include "../Assert/Assert.h"
#include <cstdint>
#include <cstring>

namespace EngineCore::Foundation
{

/**
 * @brief Pool allocator for fixed-size blocks
 * Perfect for ECS components and other uniform objects
 */
class PoolAllocator final : public IAllocator
{
public:
    /**
     * @param memory Pointer to pre-allocated memory buffer
     * @param size Total size of the buffer in bytes
     * @param blockSize Size of each block in bytes
     * @param tag Memory tag for tracking
     */
    PoolAllocator(void* memory, size_t size, size_t blockSize, MemoryTag tag = MemoryTag::ECS) noexcept
        : m_memory(static_cast<uint8_t*>(memory))
        , m_size(size)
        , m_blockSize(blockSize)
        , m_blockCount(size / blockSize)
        , m_freeList(nullptr)
        , m_tag(tag)
    {
        LT_ASSERT(memory != nullptr);
        LT_ASSERT(size > 0);
        LT_ASSERT(blockSize >= sizeof(void*)); // Block must be large enough to store pointer
        LT_ASSERT(blockSize <= size);

        // Initialize free list
        initializeFreeList();
    }

    ~PoolAllocator() override = default;

    // Non-copyable, non-movable
    PoolAllocator(const PoolAllocator&) = delete;
    PoolAllocator& operator=(const PoolAllocator&) = delete;
    PoolAllocator(PoolAllocator&&) = delete;
    PoolAllocator& operator=(PoolAllocator&&) = delete;

    [[nodiscard]] void* allocate(size_t size, size_t alignment = alignof(std::max_align_t)) override
    {
        if (size == 0)
            return nullptr;

        // Pool allocator only supports fixed block size
        if (size > m_blockSize)
        {
            return nullptr;
        }

        // Check alignment
        if (alignment > m_blockSize)
        {
            return nullptr;
        }

        if (m_freeList == nullptr)
        {
            return nullptr; // Out of memory
        }

        // Pop from free list
        void* result = m_freeList;
        m_freeList = *static_cast<void**>(m_freeList);

        return result;
    }

    void deallocate(void* ptr) noexcept override
    {
        if (ptr == nullptr || !owns(ptr))
            return;

        // Push to free list
        *static_cast<void**>(ptr) = m_freeList;
        m_freeList = ptr;
    }

    [[nodiscard]] size_t getUsed() const noexcept override
    {
        size_t freeCount = 0;
        void* current = m_freeList;
        while (current != nullptr)
        {
            freeCount++;
            current = *static_cast<void**>(current);
        }
        return (m_blockCount - freeCount) * m_blockSize;
    }

    [[nodiscard]] size_t getCapacity() const noexcept override
    {
        return m_size;
    }

    [[nodiscard]] MemoryTag getTag() const noexcept override
    {
        return m_tag;
    }

    [[nodiscard]] bool owns(void* ptr) const noexcept override
    {
        if (ptr == nullptr)
            return false;
        
        uint8_t* bytePtr = static_cast<uint8_t*>(ptr);
        if (bytePtr < m_memory || bytePtr >= (m_memory + m_size))
            return false;

        // Check if pointer is aligned to block boundary
        size_t offset = bytePtr - m_memory;
        return (offset % m_blockSize) == 0;
    }

    void reset() noexcept override
    {
        m_freeList = nullptr;
        initializeFreeList();
    }

    [[nodiscard]] size_t getBlockSize() const noexcept
    {
        return m_blockSize;
    }

    [[nodiscard]] size_t getBlockCount() const noexcept
    {
        return m_blockCount;
    }

private:
    void initializeFreeList() noexcept
    {
        m_freeList = nullptr;

        // Link all blocks in free list
        for (size_t i = 0; i < m_blockCount; ++i)
        {
            void* block = m_memory + (i * m_blockSize);
            *static_cast<void**>(block) = m_freeList;
            m_freeList = block;
        }
    }

    uint8_t* m_memory;
    size_t m_size;
    size_t m_blockSize;
    size_t m_blockCount;
    void* m_freeList;
    MemoryTag m_tag;
};

} // namespace EngineCore::Foundation

