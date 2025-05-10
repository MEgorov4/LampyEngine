#pragma once

class DoubleStackAllocator;

template<typename T>
class GarbageAllocator
{
public:
	using value_type = T;

    GarbageAllocator() noexcept = default;
	GarbageAllocator(DoubleStackAllocator* doubleStack);

    GarbageAllocator(const GarbageAllocator&) = delete;
    GarbageAllocator& operator=(const GarbageAllocator&) = delete;

    template <typename U>
    GarbageAllocator(const GarbageAllocator<U>& other)
        : m_doubleStack(other.m_doubleStack) {
    };

    T* allocate(size_t n) 
    {
        return innerAllocate(n);
    };
    void deallocate(T*, size_t) noexcept 
    {
        // do nothing, double stack allocator do this
    };

    template <typename U>
    struct rebind {
        using other = GarbageAllocator<U>;
    };

    bool operator==(const GarbageAllocator&) const noexcept { return true; }
    bool operator!=(const GarbageAllocator&) const noexcept { return false; }

    DoubleStackAllocator* m_doubleStack;

private:
    T* innerAllocate(size_t n);
};

