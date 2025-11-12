#pragma once

#include <cstddef>
#include <cstdint>

namespace EngineCore::Foundation
{

enum class MemoryTag : uint8_t
{
    Unknown,
    Render,
    Physics,
    Audio,
    UI,
    Temp,
    ECS,
    Resource,
    Script,
    Count
};

class IAllocator
{
public:
    virtual ~IAllocator() = default;

    /**
     * @brief Allocates memory with specified size and alignment
     * @param size Size in bytes to allocate
     * @param alignment Alignment requirement (must be power of 2)
     * @return Pointer to allocated memory, or nullptr if allocation failed
     */
    [[nodiscard]] virtual void* allocate(size_t size, size_t alignment = alignof(std::max_align_t)) = 0;

    /**
     * @brief Deallocates memory previously allocated by this allocator
     * @param ptr Pointer to memory to deallocate
     */
    virtual void deallocate(void* ptr) noexcept = 0;

    /**
     * @brief Get currently used memory in bytes
     */
    [[nodiscard]] virtual size_t getUsed() const noexcept = 0;

    /**
     * @brief Get total capacity in bytes
     */
    [[nodiscard]] virtual size_t getCapacity() const noexcept = 0;

    /**
     * @brief Get memory tag for this allocator
     */
    [[nodiscard]] virtual MemoryTag getTag() const noexcept = 0;

    /**
     * @brief Check if pointer belongs to this allocator
     */
    [[nodiscard]] virtual bool owns(void* ptr) const noexcept = 0;

    /**
     * @brief Reset allocator (if supported)
     */
    virtual void reset() noexcept {}
};

} // namespace EngineCore::Foundation

