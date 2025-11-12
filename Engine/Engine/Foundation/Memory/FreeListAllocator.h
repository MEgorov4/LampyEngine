#pragma once

#include "IAllocator.h"
#include "../Assert/Assert.h"
#include "../Log/LoggerMacro.h"
#include <cstdint>
#include <cstring>
#include <mutex>

namespace EngineCore::Foundation
{

/**
 * @brief Free list allocator for variable-size allocations
 * Uses first-fit strategy with coalescing
 */
class FreeListAllocator final : public IAllocator
{
private:
    struct BlockHeader
    {
        size_t size;
        BlockHeader* next;
        bool isFree;
    };

    struct Footer
    {
        size_t size;
        BlockHeader* header; // Back pointer for O(1) findHeader()
    };

    static constexpr size_t MIN_BLOCK_SIZE = sizeof(BlockHeader) + sizeof(Footer) + sizeof(void*);

    // Member variables
    uint8_t* m_memory;
    size_t m_size;
    BlockHeader* m_freeList;
    MemoryTag m_tag;
    mutable std::mutex m_mutex; // Protect m_freeList and related operations

public:
    /**
     * @param memory Pointer to pre-allocated memory buffer
     * @param size Size of the buffer in bytes
     * @param tag Memory tag for tracking
     */
    FreeListAllocator(void* memory, size_t size, MemoryTag tag = MemoryTag::Unknown) noexcept
        : m_memory(static_cast<uint8_t*>(memory))
        , m_size(size)
        , m_tag(tag)
    {
        LT_ASSERT(memory != nullptr);
        LT_ASSERT(size >= MIN_BLOCK_SIZE);

        // Initialize first free block covering entire memory
        // header->size stores the FULL block size (header + data + footer)
        BlockHeader* header = reinterpret_cast<BlockHeader*>(m_memory);
        header->size = size; // Full size including header and footer
        header->next = nullptr;
        header->isFree = true;

        Footer* footer = getFooter(header);
        footer->size = header->size;
        footer->header = header; // Set back pointer

        m_freeList = header;
    }

    ~FreeListAllocator() override = default;

    // Non-copyable, non-movable
    FreeListAllocator(const FreeListAllocator&) = delete;
    FreeListAllocator& operator=(const FreeListAllocator&) = delete;
    FreeListAllocator(FreeListAllocator&&) = delete;
    FreeListAllocator& operator=(FreeListAllocator&&) = delete;

    [[nodiscard]] void* allocate(size_t size, size_t alignment = alignof(std::max_align_t)) override
    {
        if (size == 0)
            return nullptr;

        std::lock_guard<std::mutex> lock(m_mutex);

        // Calculate required size: header + 1 byte for offset + max alignment padding + user data + footer
        // We'll calculate the actual size after finding a block, but need a conservative estimate
        size_t maxAlignmentPadding = alignment - 1;
        size_t estimatedRequiredSize = sizeof(BlockHeader) + 1 + maxAlignmentPadding + size + sizeof(Footer);
        if (estimatedRequiredSize < MIN_BLOCK_SIZE)
            estimatedRequiredSize = MIN_BLOCK_SIZE;

        // Find first fit
        BlockHeader* current = m_freeList;
        BlockHeader* previous = nullptr;

        while (current != nullptr)
        {
            if (current->isFree && current->size >= estimatedRequiredSize)
            {
                // Calculate actual alignment offset
                // Reserve 1 byte before user data for storing offset
                uint8_t* dataAreaStart = reinterpret_cast<uint8_t*>(current) + sizeof(BlockHeader) + 1;
                size_t alignmentOffset = alignUp(reinterpret_cast<uintptr_t>(dataAreaStart), alignment) - reinterpret_cast<uintptr_t>(dataAreaStart);
                
                // Calculate actual block size needed: header + 1 byte for offset + alignment padding + user data + footer
                size_t actualBlockSize = sizeof(BlockHeader) + 1 + alignmentOffset + size + sizeof(Footer);
                
                // Check if block is still large enough after alignment
                if (current->size < actualBlockSize)
                {
                    previous = current;
                    current = current->next;
                    continue;
                }

                // Found a block! Remove from free list FIRST before splitting
                // This is important because splitBlock may modify m_freeList
                if (previous == nullptr)
                    m_freeList = current->next;
                else
                    previous->next = current->next;
                
                // Mark as allocated before splitting
                current->next = nullptr;
                current->isFree = false;

                // Now check if we need to split
                size_t remainingSize = current->size - actualBlockSize;
                if (remainingSize >= MIN_BLOCK_SIZE)
                {
                    // Split block - use actualBlockSize
                    splitBlock(current, actualBlockSize);
                }
                
                // Update footer - footer is always at the end of current->size
                Footer* footer = getFooter(current);
                footer->size = current->size;
                footer->header = current; // Set back pointer

                // Calculate final user data pointer
                // The offset byte is right after the header
                uint8_t* offsetByte = reinterpret_cast<uint8_t*>(current) + sizeof(BlockHeader);
                uint8_t* userDataPtr = dataAreaStart + alignmentOffset;
                void* userData = userDataPtr;
                
                // Store total offset (1 byte reserved + alignment offset) in the offset byte
                *offsetByte = static_cast<uint8_t>(1 + alignmentOffset);

                return userData;
            }

            previous = current;
            current = current->next;
        }

        LT_LOGW("Memory", "Out of memory: no suitable block found");
        return nullptr; // Out of memory
    }

    void deallocate(void* ptr) noexcept override
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        if (ptr == nullptr || !owns(ptr))
        {
            LT_LOGW("Memory", "Attempted to deallocate invalid pointer");
            return;
        }

        // Find header (account for alignment offset)
        BlockHeader* header = findHeader(ptr);
        if (header == nullptr || header->isFree)
        {
            LT_LOGW("Memory", "Header not found or already free during deallocation");
            return;
        }

        header->isFree = true;

        // Coalesce with adjacent free blocks
        // This may change the header pointer if merged with previous block
        BlockHeader* finalHeader = coalesce(header);

        // Add to free list only if not already there
        if (!isInFreeList(finalHeader))
        {
            finalHeader->next = m_freeList;
            m_freeList = finalHeader;
        }
    }

    bool isInFreeList(BlockHeader* block) const noexcept
    {
        // Called from deallocate() which already holds the lock, so we don't need to lock again
        // But we make it const so it can be called from const contexts (though it modifies mutable state)
        BlockHeader* current = m_freeList;
        while (current != nullptr)
        {
            if (current == block)
                return true;
            current = current->next;
        }
        return false;
    }

    [[nodiscard]] size_t getUsed() const noexcept override
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        size_t used = 0;
        uint8_t* current = m_memory;
        
        while (current < m_memory + m_size)
        {
            BlockHeader* header = reinterpret_cast<BlockHeader*>(current);
            // header->size already includes the full block size (header + data + footer)
            // But we need to count only the actual data portion for used blocks
            if (!header->isFree)
            {
                // Block size includes header + data + footer, so we count the full block
                used += header->size;
            }
            // Move to next block: header->size already includes header + data + footer
            current += header->size;
        }

        return used;
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

private:
    static uintptr_t alignUp(uintptr_t value, size_t alignment) noexcept
    {
        LT_ASSERT((alignment & (alignment - 1)) == 0); // Must be power of 2
        return (value + alignment - 1) & ~(static_cast<uintptr_t>(alignment - 1));
    }

    Footer* getFooter(BlockHeader* header) const noexcept
    {
        // header->size is the full block size, so footer is at the end
        uint8_t* blockEnd = reinterpret_cast<uint8_t*>(header) + header->size;
        uint8_t* footerPtr = blockEnd - sizeof(Footer);
        
        // Safety check
        LT_ASSERT(footerPtr >= m_memory && footerPtr < m_memory + m_size);
        
        return reinterpret_cast<Footer*>(footerPtr);
    }

    BlockHeader* getPreviousBlock(BlockHeader* header) const noexcept
    {
        if (header == reinterpret_cast<BlockHeader*>(m_memory))
            return nullptr;

        // Search backwards to find previous block
        // footer->size is the full block size
        uint8_t* current = reinterpret_cast<uint8_t*>(header) - sizeof(Footer);
        Footer* footer = reinterpret_cast<Footer*>(current);
        uint8_t* prevBlockStart = reinterpret_cast<uint8_t*>(header) - footer->size;
        
        return reinterpret_cast<BlockHeader*>(prevBlockStart);
    }

    BlockHeader* getNextBlock(BlockHeader* header) const noexcept
    {
        // header->size is the full block size
        uint8_t* nextBlockStart = reinterpret_cast<uint8_t*>(header) + header->size;
        if (nextBlockStart >= m_memory + m_size)
            return nullptr;
        
        return reinterpret_cast<BlockHeader*>(nextBlockStart);
    }

    void splitBlock(BlockHeader* header, size_t blockSize) noexcept
    {
        // blockSize includes header + data + footer
        size_t remainingSize = header->size - blockSize;
        
        // Update current block size BEFORE creating new block
        header->size = blockSize;
        Footer* footer = getFooter(header);
        footer->size = blockSize;
        footer->header = header; // Update back pointer

        // Create new free block from remaining space
        if (remainingSize >= MIN_BLOCK_SIZE)
        {
            uint8_t* newBlockStart = reinterpret_cast<uint8_t*>(header) + blockSize;
            BlockHeader* newHeader = reinterpret_cast<BlockHeader*>(newBlockStart);
            newHeader->size = remainingSize;
            newHeader->isFree = true;
            newHeader->next = nullptr; // Will be set by caller if needed
            
            // Add to free list at front (simpler and faster)
            newHeader->next = m_freeList;
            m_freeList = newHeader;

            Footer* newFooter = getFooter(newHeader);
            newFooter->size = remainingSize;
            newFooter->header = newHeader; // Set back pointer
        }
    }

    BlockHeader* coalesce(BlockHeader* header) noexcept
    {
        // Coalesce with next block first
        BlockHeader* next = getNextBlock(header);
        if (next != nullptr && next->isFree)
        {
            // Remove next from free list
            removeFromFreeList(next);

            // Merge next into current
            // Both sizes are full block sizes, so we just add them
            header->size += next->size;
            Footer* footer = getFooter(header);
            footer->size = header->size;
            footer->header = header; // Update back pointer
        }

        // Coalesce with previous block
        BlockHeader* prev = getPreviousBlock(header);
        if (prev != nullptr && prev->isFree)
        {
            // Remove both from free list
            removeFromFreeList(prev);
            removeFromFreeList(header);

            // Merge current into previous
            // Both sizes are full block sizes, so we just add them
            prev->size += header->size;
            Footer* footer = getFooter(prev);
            footer->size = prev->size;
            footer->header = prev; // Update back pointer

            // Return previous block as the final header
            return prev;
        }

        return header;
    }

    void removeFromFreeList(BlockHeader* block) noexcept
    {
        if (block == nullptr)
            return;

        BlockHeader* current = m_freeList;
        BlockHeader* prev = nullptr;

        while (current != nullptr)
        {
            if (current == block)
            {
                if (prev == nullptr)
                    m_freeList = current->next;
                else
                    prev->next = current->next;
                break;
            }
            prev = current;
            current = current->next;
        }
    }

    BlockHeader* findHeader(void* ptr) const noexcept
    {
        if (ptr == nullptr)
            return nullptr;

        uint8_t* userDataPtr = static_cast<uint8_t*>(ptr);
        
        // O(n) search through all blocks to find which one contains this pointer
        // This is reliable and simple
        uint8_t* current = m_memory;
        
        while (current < m_memory + m_size)
        {
            BlockHeader* header = reinterpret_cast<BlockHeader*>(current);
            
            // Check if userDataPtr is within this block
            uint8_t* blockStart = current;
            uint8_t* blockEnd = current + header->size;
            
            if (userDataPtr >= blockStart && userDataPtr < blockEnd)
            {
                // Verify this is an allocated block
                if (!header->isFree)
                {
                    // Verify userDataPtr is in the data area (not in header, offset byte, or footer)
                    uint8_t* dataStart = blockStart + sizeof(BlockHeader) + 1; // +1 for offset byte
                    uint8_t* dataEnd = blockEnd - sizeof(Footer);
                    
                    if (userDataPtr >= dataStart && userDataPtr < dataEnd)
                    {
                        return header;
                    }
                }
            }
            
            // Move to next block
            current += header->size;
        }
        
        return nullptr;
    }
};

} // namespace EngineCore::Foundation

