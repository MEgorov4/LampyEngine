#include <gtest/gtest.h>
#include <Foundation/JobSystem/JobSystem.h>
#include <atomic>
#include <vector>
#include <mutex>
#include <numeric>
#include <algorithm>

using namespace EngineCore::Foundation;

class ParallelForTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        jobSystem = std::make_unique<JobSystem>();
        jobSystem->startup();
    }
    
    void TearDown() override
    {
        if (jobSystem)
        {
            jobSystem->shutdown();
            jobSystem.reset();
        }
    }
    
    std::unique_ptr<JobSystem> jobSystem;
};

// ============================================================================
// Basic parallel_for Tests
// ============================================================================

TEST_F(ParallelForTest, BasicParallelFor)
{
    const size_t size = 1000;
    std::vector<int> data(size, 0);
    
    jobSystem->parallel_for(0, size, [&data](size_t i) {
        data[i] = static_cast<int>(i);
    });
    
    for (size_t i = 0; i < size; ++i)
    {
        EXPECT_EQ(data[i], static_cast<int>(i)) << "Index " << i;
    }
}

TEST_F(ParallelForTest, ParallelForWithoutStartup)
{
    // Should run sequentially if not started
    JobSystem localJobSystem;
    
    const size_t size = 100;
    std::vector<int> data(size, 0);
    
    localJobSystem.parallel_for(0, size, [&data](size_t i) {
        data[i] = static_cast<int>(i);
    });
    
    for (size_t i = 0; i < size; ++i)
    {
        EXPECT_EQ(data[i], static_cast<int>(i));
    }
}

TEST_F(ParallelForTest, EmptyRange)
{
    std::vector<int> data(10, 0);
    
    // Should not crash
    jobSystem->parallel_for(0, 0, [&data](size_t i) {
        data[i] = 1;
    });
    
    // Data should remain unchanged
    for (int val : data)
    {
        EXPECT_EQ(val, 0);
    }
}

TEST_F(ParallelForTest, SingleElementRange)
{
    std::atomic<int> counter{0};
    
    jobSystem->parallel_for(0, 1, [&counter](size_t i) {
        counter.fetch_add(1);
    });
    
    EXPECT_EQ(counter.load(), 1);
}

TEST_F(ParallelForTest, SmallRange)
{
    const size_t size = 10;
    std::vector<int> data(size, 0);
    
    jobSystem->parallel_for(0, size, [&data](size_t i) {
        data[i] = static_cast<int>(i * 2);
    });
    
    for (size_t i = 0; i < size; ++i)
    {
        EXPECT_EQ(data[i], static_cast<int>(i * 2));
    }
}

TEST_F(ParallelForTest, LargeRange)
{
    const size_t size = 10000;
    std::vector<int> data(size, 0);
    
    jobSystem->parallel_for(0, size, [&data](size_t i) {
        data[i] = static_cast<int>(i);
    });
    
    for (size_t i = 0; i < size; ++i)
    {
        EXPECT_EQ(data[i], static_cast<int>(i));
    }
}

// ============================================================================
// Custom Grain Size Tests
// ============================================================================

TEST_F(ParallelForTest, CustomGrainSize)
{
    const size_t size = 1000;
    const size_t grain = 100;
    std::vector<int> data(size, 0);
    
    jobSystem->parallel_for(0, size, [&data](size_t i) {
        data[i] = static_cast<int>(i);
    }, grain);
    
    for (size_t i = 0; i < size; ++i)
    {
        EXPECT_EQ(data[i], static_cast<int>(i));
    }
}

TEST_F(ParallelForTest, LargeGrainSize)
{
    const size_t size = 1000;
    const size_t grain = 500;
    std::vector<int> data(size, 0);
    
    jobSystem->parallel_for(0, size, [&data](size_t i) {
        data[i] = static_cast<int>(i);
    }, grain);
    
    for (size_t i = 0; i < size; ++i)
    {
        EXPECT_EQ(data[i], static_cast<int>(i));
    }
}

TEST_F(ParallelForTest, SmallGrainSize)
{
    const size_t size = 1000;
    const size_t grain = 1;
    std::vector<int> data(size, 0);
    
    jobSystem->parallel_for(0, size, [&data](size_t i) {
        data[i] = static_cast<int>(i);
    }, grain);
    
    for (size_t i = 0; i < size; ++i)
    {
        EXPECT_EQ(data[i], static_cast<int>(i));
    }
}

// ============================================================================
// Non-Zero Start Index Tests
// ============================================================================

TEST_F(ParallelForTest, NonZeroStart)
{
    const size_t start = 100;
    const size_t end = 200;
    std::vector<int> data(300, 0);
    
    jobSystem->parallel_for(start, end, [&data](size_t i) {
        data[i] = static_cast<int>(i);
    });
    
    for (size_t i = 0; i < start; ++i)
    {
        EXPECT_EQ(data[i], 0);
    }
    
    for (size_t i = start; i < end; ++i)
    {
        EXPECT_EQ(data[i], static_cast<int>(i));
    }
    
    for (size_t i = end; i < data.size(); ++i)
    {
        EXPECT_EQ(data[i], 0);
    }
}

TEST_F(ParallelForTest, OffsetRange)
{
    const size_t start = 50;
    const size_t end = 150;
    std::vector<int> data(200, 0);
    
    jobSystem->parallel_for(start, end, [&data](size_t i) {
        data[i] = static_cast<int>(i * 2);
    });
    
    for (size_t i = start; i < end; ++i)
    {
        EXPECT_EQ(data[i], static_cast<int>(i * 2));
    }
}

// ============================================================================
// Thread Safety Tests
// ============================================================================

TEST_F(ParallelForTest, ThreadSafeIncrement)
{
    const size_t size = 10000;
    std::atomic<size_t> counter{0};
    
    jobSystem->parallel_for(0, size, [&counter](size_t i) {
        counter.fetch_add(1);
    });
    
    EXPECT_EQ(counter.load(), size);
}

TEST_F(ParallelForTest, ThreadSafeAccumulation)
{
    const size_t size = 1000;
    std::atomic<size_t> sum{0};
    
    jobSystem->parallel_for(0, size, [&sum](size_t i) {
        sum.fetch_add(i);
    });
    
    size_t expected = (size - 1) * size / 2;
    EXPECT_EQ(sum.load(), expected);
}

TEST_F(ParallelForTest, MutexProtectedAccess)
{
    const size_t size = 1000;
    std::vector<int> data(size, 0);
    std::mutex mutex;
    
    jobSystem->parallel_for(0, size, [&data, &mutex](size_t i) {
        std::lock_guard<std::mutex> lock(mutex);
        data[i] = static_cast<int>(i);
    });
    
    for (size_t i = 0; i < size; ++i)
    {
        EXPECT_EQ(data[i], static_cast<int>(i));
    }
}

// ============================================================================
// Performance and Correctness Tests
// ============================================================================

TEST_F(ParallelForTest, AllElementsProcessed)
{
    const size_t size = 5000;
    std::vector<std::atomic<bool>> processed(size);
    
    for (size_t i = 0; i < size; ++i)
    {
        processed[i] = false;
    }
    
    jobSystem->parallel_for(0, size, [&processed](size_t i) {
        processed[i].store(true);
    });
    
    for (size_t i = 0; i < size; ++i)
    {
        EXPECT_TRUE(processed[i].load()) << "Element " << i << " was not processed";
    }
}

TEST_F(ParallelForTest, NoDoubleProcessing)
{
    const size_t size = 1000;
    std::vector<std::atomic<int>> counts(size);
    
    for (size_t i = 0; i < size; ++i)
    {
        counts[i] = 0;
    }
    
    jobSystem->parallel_for(0, size, [&counts](size_t i) {
        counts[i].fetch_add(1);
    });
    
    for (size_t i = 0; i < size; ++i)
    {
        EXPECT_EQ(counts[i].load(), 1) << "Element " << i << " processed " << counts[i].load() << " times";
    }
}

TEST_F(ParallelForTest, CorrectOrderIndependent)
{
    const size_t size = 1000;
    std::vector<int> data(size, 0);
    
    jobSystem->parallel_for(0, size, [&data](size_t i) {
        data[i] = static_cast<int>(i * i);
    });
    
    for (size_t i = 0; i < size; ++i)
    {
        EXPECT_EQ(data[i], static_cast<int>(i * i));
    }
}

// ============================================================================
// Complex Operations Tests
// ============================================================================

TEST_F(ParallelForTest, ComplexComputation)
{
    const size_t size = 1000;
    std::vector<double> results(size);
    
    jobSystem->parallel_for(0, size, [&results](size_t i) {
        double sum = 0.0;
        for (size_t j = 0; j < 100; ++j)
        {
            sum += std::sin(static_cast<double>(i + j));
        }
        results[i] = sum;
    });
    
    // Verify all computed
    for (size_t i = 0; i < size; ++i)
    {
        EXPECT_NE(results[i], 0.0);
    }
}

TEST_F(ParallelForTest, NestedParallelFor)
{
    const size_t outerSize = 100;
    const size_t innerSize = 100;
    std::vector<std::vector<int>> matrix(outerSize, std::vector<int>(innerSize, 0));
    
    jobSystem->parallel_for(0, outerSize, [&matrix, innerSize, this](size_t i) {
        jobSystem->parallel_for(0, innerSize, [&matrix, i, innerSize](size_t j) {
            matrix[i][j] = static_cast<int>(i * innerSize + j);
        });
    });
    
    for (size_t i = 0; i < outerSize; ++i)
    {
        for (size_t j = 0; j < innerSize; ++j)
        {
            EXPECT_EQ(matrix[i][j], static_cast<int>(i * innerSize + j));
        }
    }
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(ParallelForTest, BeginGreaterThanEnd)
{
    std::vector<int> data(10, 0);
    
    // Should not crash and should not modify data
    jobSystem->parallel_for(10, 5, [&data](size_t i) {
        data[i] = 1;
    });
    
    for (int val : data)
    {
        EXPECT_EQ(val, 0);
    }
}

TEST_F(ParallelForTest, VeryLargeRange)
{
    const size_t size = 100000;
    std::atomic<size_t> counter{0};
    
    jobSystem->parallel_for(0, size, [&counter](size_t i) {
        counter.fetch_add(1);
    });
    
    EXPECT_EQ(counter.load(), size);
}

TEST_F(ParallelForTest, LambdaWithCapture)
{
    const size_t size = 1000;
    int multiplier = 5;
    std::vector<int> data(size, 0);
    
    jobSystem->parallel_for(0, size, [&data, multiplier](size_t i) {
        data[i] = static_cast<int>(i) * multiplier;
    });
    
    for (size_t i = 0; i < size; ++i)
    {
        EXPECT_EQ(data[i], static_cast<int>(i) * multiplier);
    }
}

TEST_F(ParallelForTest, LambdaThrowsException)
{
    const size_t size = 100;
    std::vector<int> data(size, 0);
    
    // Should not crash, but some elements may not be processed
    try
    {
        jobSystem->parallel_for(0, size, [&data](size_t i) {
            if (i == 50)
            {
                throw std::runtime_error("Test exception");
            }
            data[i] = static_cast<int>(i);
        });
    }
    catch (...)
    {
        // Exception handling depends on implementation
    }
}

