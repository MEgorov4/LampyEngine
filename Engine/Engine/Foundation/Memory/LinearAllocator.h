#pragma once

#include "IAllocator.h"
#include "../Assert/Assert.h"
#include <cstring>
#include <cassert>
#include <cstdint>

namespace EngineCore::Foundation
{

/**
 * @brief Fast sequential allocator that can only be reset as a whole
 * Perfect for frame allocations, temporary data, and asset loading
 */
class LinearAllocator final : public IAllocator
{
public:
    /**
     * @param memory Pointer to pre-allocated memory buffer
     * @param size Size of the buffer in bytes
     * @param tag Memory tag for tracking
     */
    LinearAllocator(void* memory, size_t size, MemoryTag tag = MemoryTag::Temp) noexcept
        : m_memory(static_cast<uint8_t*>(memory))
        , m_size(size)
        , m_used(0)
        , m_tag(tag)
    {
        LT_ASSERT(memory != nullptr);
        LT_ASSERT(size > 0);
    }

    ~LinearAllocator() override = default;

    // Non-copyable, non-movable
    LinearAllocator(const LinearAllocator&) = delete;
    LinearAllocator& operator=(const LinearAllocator&) = delete;
    LinearAllocator(LinearAllocator&&) = delete;
    LinearAllocator& operator=(LinearAllocator&&) = delete;

    [[nodiscard]] void* allocate(size_t size, size_t alignment = alignof(std::max_align_t)) override
    {
        if (size == 0)
            return nullptr;

        // Calculate aligned pointer from the current position
        // We need to align from the base address + used offset
        uintptr_t baseAddr = reinterpret_cast<uintptr_t>(m_memory);
        uintptr_t currentAddr = baseAddr + m_used;
        uintptr_t alignedAddr = alignUp(currentAddr, alignment);
        size_t padding = alignedAddr - currentAddr;
        size_t totalSize = padding + size;

        if (m_used + totalSize > m_size)
        {
            return nullptr; // Out of memory
        }

        void* result = reinterpret_cast<void*>(alignedAddr);
        m_used += totalSize;

        return result;
    }

    void deallocate(void* ptr) noexcept override
    {
        // Linear allocator doesn't support individual deallocation
        // Memory is freed when reset() is called
        (void)ptr;
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
        return bytePtr >= m_memory && bytePtr < (m_memory + m_size);
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

