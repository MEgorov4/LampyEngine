#include <gtest/gtest.h>
#include <Foundation/Memory/IAllocator.h>
#include <Foundation/Memory/LinearAllocator.h>
#include <Foundation/Memory/StackAllocator.h>
#include <Foundation/Memory/PoolAllocator.h>
#include <Foundation/Memory/FreeListAllocator.h>
#include <Foundation/Memory/MemorySystem.h>
#include <cstring>
#include <vector>
#include <random>

using namespace EngineCore::Foundation;

class AllocatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Initialize memory system for tests
        MemorySystem::startup(1024 * 1024, 4 * 1024 * 1024); // 1MB frame, 4MB persistent
    }

    void TearDown() override
    {
        MemorySystem::shutdown();
    }
};

// LinearAllocator Tests
TEST_F(AllocatorTest, LinearAllocator_BasicAllocation)
{
    constexpr size_t size = 1024;
    auto memory = std::make_unique<uint8_t[]>(size);
    LinearAllocator allocator(memory.get(), size, MemoryTag::Temp);

    void* ptr1 = allocator.allocate(64);
    void* ptr2 = allocator.allocate(128);

    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);
    ASSERT_NE(ptr1, ptr2);
    ASSERT_EQ(allocator.getUsed(), 64 + 128);
}

TEST_F(AllocatorTest, LinearAllocator_Alignment)
{
    constexpr size_t size = 1024;
    auto memory = std::make_unique<uint8_t[]>(size);
    LinearAllocator allocator(memory.get(), size, MemoryTag::Temp);

    void* ptr1 = allocator.allocate(1, 1);
    void* ptr2 = allocator.allocate(1, 16);
    void* ptr3 = allocator.allocate(1, 32);

    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);
    ASSERT_NE(ptr3, nullptr);

    // Check alignment
    ASSERT_EQ(reinterpret_cast<uintptr_t>(ptr2) % 16, 0);
    ASSERT_EQ(reinterpret_cast<uintptr_t>(ptr3) % 32, 0);
}

TEST_F(AllocatorTest, LinearAllocator_Reset)
{
    constexpr size_t size = 1024;
    auto memory = std::make_unique<uint8_t[]>(size);
    LinearAllocator allocator(memory.get(), size, MemoryTag::Temp);

    allocator.allocate(512);
    ASSERT_GT(allocator.getUsed(), 0);

    allocator.reset();
    ASSERT_EQ(allocator.getUsed(), 0);
}

TEST_F(AllocatorTest, LinearAllocator_OutOfMemory)
{
    constexpr size_t size = 100;
    auto memory = std::make_unique<uint8_t[]>(size);
    LinearAllocator allocator(memory.get(), size, MemoryTag::Temp);

    void* ptr1 = allocator.allocate(50);
    ASSERT_NE(ptr1, nullptr);

    void* ptr2 = allocator.allocate(60); // Should fail
    ASSERT_EQ(ptr2, nullptr);
}

// StackAllocator Tests
TEST_F(AllocatorTest, StackAllocator_Markers)
{
    constexpr size_t size = 1024;
    auto memory = std::make_unique<uint8_t[]>(size);
    StackAllocator allocator(memory.get(), size, MemoryTag::Temp);

    void* ptr1 = allocator.allocate(64);
    StackAllocator::Marker marker = allocator.getMarker();
    void* ptr2 = allocator.allocate(128);

    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);
    ASSERT_GT(allocator.getUsed(), 0);

    allocator.rollbackToMarker(marker);
    ASSERT_EQ(allocator.getUsed(), marker);
}

// PoolAllocator Tests
TEST_F(AllocatorTest, PoolAllocator_BasicAllocation)
{
    constexpr size_t poolSize = 1024;
    constexpr size_t blockSize = 64;
    auto memory = std::make_unique<uint8_t[]>(poolSize);
    PoolAllocator allocator(memory.get(), poolSize, blockSize, MemoryTag::ECS);

    std::vector<void*> ptrs;
    for (size_t i = 0; i < 10; ++i)
    {
        void* ptr = allocator.allocate(blockSize);
        ASSERT_NE(ptr, nullptr);
        ptrs.push_back(ptr);
    }

    // All should be different
    for (size_t i = 0; i < ptrs.size(); ++i)
    {
        for (size_t j = i + 1; j < ptrs.size(); ++j)
        {
            ASSERT_NE(ptrs[i], ptrs[j]);
        }
    }
}

TEST_F(AllocatorTest, PoolAllocator_Deallocation)
{
    constexpr size_t poolSize = 1024;
    constexpr size_t blockSize = 64;
    auto memory = std::make_unique<uint8_t[]>(poolSize);
    PoolAllocator allocator(memory.get(), poolSize, blockSize, MemoryTag::ECS);

    void* ptr1 = allocator.allocate(blockSize);
    void* ptr2 = allocator.allocate(blockSize);
    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);

    allocator.deallocate(ptr1);
    allocator.deallocate(ptr2);

    // Should be able to allocate again
    void* ptr3 = allocator.allocate(blockSize);
    void* ptr4 = allocator.allocate(blockSize);
    ASSERT_NE(ptr3, nullptr);
    ASSERT_NE(ptr4, nullptr);
}

TEST_F(AllocatorTest, PoolAllocator_TooLarge)
{
    constexpr size_t poolSize = 1024;
    constexpr size_t blockSize = 64;
    auto memory = std::make_unique<uint8_t[]>(poolSize);
    PoolAllocator allocator(memory.get(), poolSize, blockSize, MemoryTag::ECS);

    void* ptr = allocator.allocate(blockSize + 1); // Too large
    ASSERT_EQ(ptr, nullptr);
}

// FreeListAllocator Tests
TEST_F(AllocatorTest, FreeListAllocator_BasicAllocation)
{
    constexpr size_t size = 4096;
    auto memory = std::make_unique<uint8_t[]>(size);
    FreeListAllocator allocator(memory.get(), size, MemoryTag::Unknown);

    void* ptr1 = allocator.allocate(64);
    void* ptr2 = allocator.allocate(128);
    void* ptr3 = allocator.allocate(256);

    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);
    ASSERT_NE(ptr3, nullptr);
    ASSERT_NE(ptr1, ptr2);
    ASSERT_NE(ptr2, ptr3);
}

TEST_F(AllocatorTest, FreeListAllocator_Deallocation)
{
    constexpr size_t size = 4096;
    auto memory = std::make_unique<uint8_t[]>(size);
    FreeListAllocator allocator(memory.get(), size, MemoryTag::Unknown);

    void* ptr1 = allocator.allocate(64);
    void* ptr2 = allocator.allocate(128);
    void* ptr3 = allocator.allocate(256);

    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);
    ASSERT_NE(ptr3, nullptr);

    allocator.deallocate(ptr2);
    allocator.deallocate(ptr1);
    allocator.deallocate(ptr3);

    // Should be able to allocate again
    void* ptr4 = allocator.allocate(200);
    ASSERT_NE(ptr4, nullptr);
}

TEST_F(AllocatorTest, FreeListAllocator_Alignment)
{
    constexpr size_t size = 4096;
    auto memory = std::make_unique<uint8_t[]>(size);
    FreeListAllocator allocator(memory.get(), size, MemoryTag::Unknown);

    void* ptr1 = allocator.allocate(1, 16);
    void* ptr2 = allocator.allocate(1, 32);
    void* ptr3 = allocator.allocate(1, 64);

    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);
    ASSERT_NE(ptr3, nullptr);

    ASSERT_EQ(reinterpret_cast<uintptr_t>(ptr1) % 16, 0);
    ASSERT_EQ(reinterpret_cast<uintptr_t>(ptr2) % 32, 0);
    ASSERT_EQ(reinterpret_cast<uintptr_t>(ptr3) % 64, 0);
}

// MemorySystem Tests
TEST_F(AllocatorTest, MemorySystem_FrameAllocator)
{
    IAllocator& frameAlloc = MemorySystem::getFrameAllocator();
    
    void* ptr1 = frameAlloc.allocate(64);
    void* ptr2 = frameAlloc.allocate(128);
    
    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);
    
    MemorySystem::resetFrameAllocator();
    ASSERT_EQ(frameAlloc.getUsed(), 0);
}

TEST_F(AllocatorTest, MemorySystem_PersistentAllocator)
{
    IAllocator& persistentAlloc = MemorySystem::getPersistentAllocator();
    
    void* ptr1 = persistentAlloc.allocate(64);
    void* ptr2 = persistentAlloc.allocate(128);
    
    ASSERT_NE(ptr1, nullptr);
    ASSERT_NE(ptr2, nullptr);
    
    persistentAlloc.deallocate(ptr1);
    persistentAlloc.deallocate(ptr2);
}

// Stress Tests
TEST_F(AllocatorTest, StressTest_LinearAllocator)
{
    constexpr size_t size = 1024 * 1024; // 1MB
    auto memory = std::make_unique<uint8_t[]>(size);
    LinearAllocator allocator(memory.get(), size, MemoryTag::Temp);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dis(1, 1024);

    for (int i = 0; i < 1000; ++i)
    {
        size_t allocSize = dis(gen);
        void* ptr = allocator.allocate(allocSize);
        if (ptr == nullptr)
            break; // Out of memory, expected
    }
}

TEST_F(AllocatorTest, StressTest_PoolAllocator)
{
    constexpr size_t poolSize = 1024 * 1024; // 1MB
    constexpr size_t blockSize = 64;
    auto memory = std::make_unique<uint8_t[]>(poolSize);
    PoolAllocator allocator(memory.get(), poolSize, blockSize, MemoryTag::ECS);

    std::vector<void*> ptrs;
    constexpr size_t maxAllocs = poolSize / blockSize;

    // Allocate all blocks
    for (size_t i = 0; i < maxAllocs; ++i)
    {
        void* ptr = allocator.allocate(blockSize);
        ASSERT_NE(ptr, nullptr);
        ptrs.push_back(ptr);
    }

    // Try one more - should fail
    void* ptr = allocator.allocate(blockSize);
    ASSERT_EQ(ptr, nullptr);

    // Deallocate all
    for (void* p : ptrs)
    {
        allocator.deallocate(p);
    }

    // Should be able to allocate again
    for (size_t i = 0; i < maxAllocs; ++i)
    {
        void* ptr = allocator.allocate(blockSize);
        ASSERT_NE(ptr, nullptr);
    }
}

TEST_F(AllocatorTest, StressTest_FreeListAllocator)
{
    constexpr size_t size = 1024 * 1024; // 1MB
    auto memory = std::make_unique<uint8_t[]>(size);
    FreeListAllocator allocator(memory.get(), size, MemoryTag::Unknown);

    std::vector<void*> ptrs;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dis(16, 512);

    // Allocate many blocks
    for (int i = 0; i < 100; ++i)
    {
        size_t allocSize = dis(gen);
        void* ptr = allocator.allocate(allocSize);
        if (ptr != nullptr)
        {
            ptrs.push_back(ptr);
        }
    }

    // Deallocate randomly
    std::shuffle(ptrs.begin(), ptrs.end(), gen);
    for (void* ptr : ptrs)
    {
        allocator.deallocate(ptr);
    }

    // Should be able to allocate again
    for (int i = 0; i < 50; ++i)
    {
        size_t allocSize = dis(gen);
        void* ptr = allocator.allocate(allocSize);
        ASSERT_NE(ptr, nullptr);
    }
}

