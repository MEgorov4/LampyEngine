#pragma once

#include "IAllocator.h"
#include "../Assert/Assert.h"
#include <cstdint>
#include <cstring>

namespace EngineCore::Foundation
{

/**
 * @brief Stack allocator that supports markers for rollback
 * Allows deallocation in LIFO order or rollback to a marker
 */
class StackAllocator final : public IAllocator
{
public:
    using Marker = size_t;

    /**
     * @param memory Pointer to pre-allocated memory buffer
     * @param size Size of the buffer in bytes
     * @param tag Memory tag for tracking
     */
    StackAllocator(void* memory, size_t size, MemoryTag tag = MemoryTag::Temp) noexcept
        : m_memory(static_cast<uint8_t*>(memory))
        , m_size(size)
        , m_used(0)
        , m_tag(tag)
    {
        LT_ASSERT(memory != nullptr);
        LT_ASSERT(size > 0);
    }

    ~StackAllocator() override = default;

    // Non-copyable, non-movable
    StackAllocator(const StackAllocator&) = delete;
    StackAllocator& operator=(const StackAllocator&) = delete;
    StackAllocator(StackAllocator&&) = delete;
    StackAllocator& operator=(StackAllocator&&) = delete;

    [[nodiscard]] void* allocate(size_t size, size_t alignment = alignof(std::max_align_t)) override
    {
        if (size == 0)
            return nullptr;

        // Store alignment in header (we need at least 1 byte for alignment)
        constexpr size_t headerSize = sizeof(uint8_t);
        size_t alignedOffset = alignUp(m_used + headerSize, alignment);
        size_t totalSize = alignedOffset - m_used + size; // Include the actual size

        if (m_used + totalSize > m_size)
        {
            return nullptr; // Out of memory
        }

        // Store alignment offset in header
        uint8_t alignmentOffset = static_cast<uint8_t>(alignedOffset - m_used - headerSize);
        m_memory[m_used] = alignmentOffset;

        void* result = m_memory + alignedOffset;
        m_used = alignedOffset + size;

        return result;
    }

    void deallocate(void* ptr) noexcept override
    {
        // Stack allocator only supports LIFO deallocation
        // Individual deallocation is not supported - use rollbackToMarker instead
        (void)ptr;
    }

    /**
     * @brief Get current marker position
     */
    [[nodiscard]] Marker getMarker() const noexcept
    {
        return m_used;
    }

    /**
     * @brief Rollback to a previous marker
     */
    void rollbackToMarker(Marker marker) noexcept
    {
        LT_ASSERT(marker <= m_used);
        m_used = marker;
    }

    [[nodiscard]] size_t getUsed() const noexcept override
    {
        return m_used;
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
        return bytePtr >= m_memory && bytePtr < (m_memory + m_used);
    }

    void reset() noexcept override
    {
        m_used = 0;
    }

private:
    static size_t alignUp(size_t value, size_t alignment) noexcept
    {
        LT_ASSERT((alignment & (alignment - 1)) == 0); // Must be power of 2
        return (value + alignment - 1) & ~(alignment - 1);
    }

    uint8_t* m_memory;
    size_t m_size;
    size_t m_used;
    MemoryTag m_tag;
};

} // namespace EngineCore::Foundation

