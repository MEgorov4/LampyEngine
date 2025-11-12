#include <gtest/gtest.h>
#include <Foundation/JobSystem/JobSystem.h>
#include <atomic>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>

using namespace EngineCore::Foundation;

class WorkStealingTest : public ::testing::Test
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
// Work Stealing Tests
// ============================================================================

TEST_F(WorkStealingTest, WorkStealingOccurs)
{
    // Submit many jobs to potentially overload one worker
    const size_t jobCount = 100;
    std::vector<std::atomic<bool>> executed(jobCount);
    std::vector<JobHandle> handles;
    
    for (size_t i = 0; i < jobCount; ++i)
    {
        executed[i] = false;
        handles.push_back(jobSystem->submit([&executed, i]() {
            // Small delay to allow work stealing
            std::this_thread::sleep_for(std::chrono::microseconds(100));
            executed[i] = true;
        }));
    }
    
    // Wait for all jobs
    for (auto& handle : handles)
    {
        handle.wait();
    }
    
    // Verify all executed
    for (size_t i = 0; i < jobCount; ++i)
    {
        EXPECT_TRUE(executed[i].load()) << "Job " << i << " was not executed";
    }
}

TEST_F(WorkStealingTest, UnevenWorkDistribution)
{
    // Submit jobs that take different amounts of time
    const size_t fastJobCount = 50;
    const size_t slowJobCount = 10;
    std::atomic<size_t> fastExecuted{0};
    std::atomic<size_t> slowExecuted{0};
    std::vector<JobHandle> handles;
    
    // Fast jobs
    for (size_t i = 0; i < fastJobCount; ++i)
    {
        handles.push_back(jobSystem->submit([&fastExecuted]() {
            fastExecuted.fetch_add(1);
        }));
    }
    
    // Slow jobs
    for (size_t i = 0; i < slowJobCount; ++i)
    {
        handles.push_back(jobSystem->submit([&slowExecuted]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            slowExecuted.fetch_add(1);
        }));
    }
    
    // Wait for all
    for (auto& handle : handles)
    {
        handle.wait();
    }
    
    EXPECT_EQ(fastExecuted.load(), fastJobCount);
    EXPECT_EQ(slowExecuted.load(), slowJobCount);
}

TEST_F(WorkStealingTest, ManySmallJobs)
{
    // Submit many small jobs to test work stealing efficiency
    const size_t jobCount = 500;
    std::atomic<size_t> counter{0};
    std::vector<JobHandle> handles;
    
    for (size_t i = 0; i < jobCount; ++i)
    {
        handles.push_back(jobSystem->submit([&counter]() {
            counter.fetch_add(1);
        }));
    }
    
    for (auto& handle : handles)
    {
        handle.wait();
    }
    
    EXPECT_EQ(counter.load(), jobCount);
}

TEST_F(WorkStealingTest, BurstSubmission)
{
    // Submit many jobs at once to test work stealing under load
    const size_t burstSize = 200;
    std::atomic<size_t> executed{0};
    std::vector<JobHandle> handles;
    handles.reserve(burstSize);
    
    // Submit all at once
    for (size_t i = 0; i < burstSize; ++i)
    {
        handles.push_back(jobSystem->submit([&executed]() {
            executed.fetch_add(1);
        }));
    }
    
    // Wait for all
    for (auto& handle : handles)
    {
        handle.wait();
    }
    
    EXPECT_EQ(executed.load(), burstSize);
}

// ============================================================================
// Load Balancing Tests
// ============================================================================

TEST_F(WorkStealingTest, LoadBalancing)
{
    const size_t jobCount = 100;
    std::atomic<size_t> totalExecuted{0};
    std::vector<JobHandle> handles;
    
    // Submit jobs with varying execution times
    for (size_t i = 0; i < jobCount; ++i)
    {
        handles.push_back(jobSystem->submit([&totalExecuted, i]() {
            // Vary execution time
            std::this_thread::sleep_for(std::chrono::microseconds((i % 10) * 10));
            totalExecuted.fetch_add(1);
        }));
    }
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (auto& handle : handles)
    {
        handle.wait();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    EXPECT_EQ(totalExecuted.load(), jobCount);
    // With work stealing, should complete faster than sequential execution
    EXPECT_LT(duration.count(), 1000); // Should complete in reasonable time
}

TEST_F(WorkStealingTest, RoundRobinDistribution)
{
    // Test that jobs are distributed across workers
    const size_t jobCount = 50;
    std::vector<std::atomic<bool>> executed(jobCount);
    std::vector<JobHandle> handles;
    
    for (size_t i = 0; i < jobCount; ++i)
    {
        executed[i] = false;
        handles.push_back(jobSystem->submit([&executed, i]() {
            executed[i] = true;
        }));
    }
    
    for (auto& handle : handles)
    {
        handle.wait();
    }
    
    for (size_t i = 0; i < jobCount; ++i)
    {
        EXPECT_TRUE(executed[i].load()) << "Job " << i << " not executed";
    }
}

// ============================================================================
// Concurrent Submission and Execution Tests
// ============================================================================

TEST_F(WorkStealingTest, ConcurrentSubmissionDuringExecution)
{
    std::atomic<size_t> totalExecuted{0};
    std::vector<std::thread> submitThreads;
    const size_t threadsCount = 4;
    const size_t jobsPerThread = 25;
    
    // Threads that submit jobs concurrently
    for (size_t t = 0; t < threadsCount; ++t)
    {
        submitThreads.emplace_back([this, &totalExecuted, jobsPerThread]() {
            std::vector<JobHandle> handles;
            for (size_t i = 0; i < jobsPerThread; ++i)
            {
                handles.push_back(jobSystem->submit([&totalExecuted]() {
                    totalExecuted.fetch_add(1);
                }));
            }
            
            for (auto& handle : handles)
            {
                handle.wait();
            }
        });
    }
    
    for (auto& thread : submitThreads)
    {
        thread.join();
    }
    
    EXPECT_EQ(totalExecuted.load(), threadsCount * jobsPerThread);
}

TEST_F(WorkStealingTest, MixedJobSizes)
{
    const size_t smallJobCount = 100;
    const size_t mediumJobCount = 50;
    const size_t largeJobCount = 20;
    
    std::atomic<size_t> smallExecuted{0};
    std::atomic<size_t> mediumExecuted{0};
    std::atomic<size_t> largeExecuted{0};
    
    std::vector<JobHandle> handles;
    
    // Small jobs
    for (size_t i = 0; i < smallJobCount; ++i)
    {
        handles.push_back(jobSystem->submit([&smallExecuted]() {
            smallExecuted.fetch_add(1);
        }));
    }
    
    // Medium jobs
    for (size_t i = 0; i < mediumJobCount; ++i)
    {
        handles.push_back(jobSystem->submit([&mediumExecuted]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            mediumExecuted.fetch_add(1);
        }));
    }
    
    // Large jobs
    for (size_t i = 0; i < largeJobCount; ++i)
    {
        handles.push_back(jobSystem->submit([&largeExecuted]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
            largeExecuted.fetch_add(1);
        }));
    }
    
    for (auto& handle : handles)
    {
        handle.wait();
    }
    
    EXPECT_EQ(smallExecuted.load(), smallJobCount);
    EXPECT_EQ(mediumExecuted.load(), mediumJobCount);
    EXPECT_EQ(largeExecuted.load(), largeJobCount);
}

// ============================================================================
// Stress Tests
// ============================================================================

TEST_F(WorkStealingTest, StressTestManyJobs)
{
    const size_t jobCount = 1000;
    std::atomic<size_t> counter{0};
    std::vector<JobHandle> handles;
    handles.reserve(jobCount);
    
    for (size_t i = 0; i < jobCount; ++i)
    {
        handles.push_back(jobSystem->submit([&counter]() {
            counter.fetch_add(1);
        }));
    }
    
    for (auto& handle : handles)
    {
        handle.wait();
    }
    
    EXPECT_EQ(counter.load(), jobCount);
}

TEST_F(WorkStealingTest, StressTestRapidSubmission)
{
    std::atomic<size_t> counter{0};
    const size_t batchCount = 10;
    const size_t jobsPerBatch = 100;
    
    for (size_t batch = 0; batch < batchCount; ++batch)
    {
        std::vector<JobHandle> handles;
        handles.reserve(jobsPerBatch);
        
        // Rapid submission
        for (size_t i = 0; i < jobsPerBatch; ++i)
        {
            handles.push_back(jobSystem->submit([&counter]() {
                counter.fetch_add(1);
            }));
        }
        
        // Wait for batch
        for (auto& handle : handles)
        {
            handle.wait();
        }
    }
    
    EXPECT_EQ(counter.load(), batchCount * jobsPerBatch);
}

// ============================================================================
// Worker Count Tests
// ============================================================================

TEST_F(WorkStealingTest, WorkerCount)
{
    size_t workerCount = jobSystem->getWorkerCount();
    EXPECT_GT(workerCount, 0);
    
    // Should have at least one worker (hardware_concurrency - 1, minimum 1)
    EXPECT_GE(workerCount, 1);
}

TEST_F(WorkStealingTest, WorkerCountAfterShutdown)
{
    size_t workerCountBefore = jobSystem->getWorkerCount();
    EXPECT_GT(workerCountBefore, 0);
    
    jobSystem->shutdown();
    
    size_t workerCountAfter = jobSystem->getWorkerCount();
    EXPECT_EQ(workerCountAfter, 0);
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST_F(WorkStealingTest, IntegrationWithSharedHandles)
{
    const size_t jobGroupCount = 10;
    const size_t jobsPerGroup = 20;
    
    std::vector<JobHandle> groupHandles;
    std::atomic<size_t> totalExecuted{0};
    
    for (size_t group = 0; group < jobGroupCount; ++group)
    {
        JobHandle groupHandle;
        
        for (size_t i = 0; i < jobsPerGroup; ++i)
        {
            jobSystem->submit([&totalExecuted]() {
                totalExecuted.fetch_add(1);
            }, groupHandle);
        }
        
        groupHandles.push_back(std::move(groupHandle));
    }
    
    for (auto& handle : groupHandles)
    {
        handle.wait();
    }
    
    EXPECT_EQ(totalExecuted.load(), jobGroupCount * jobsPerGroup);
}

TEST_F(WorkStealingTest, IntegrationWithParallelFor)
{
    const size_t size = 1000;
    std::atomic<size_t> counter{0};
    
    // Use parallel_for
    jobSystem->parallel_for(0, size, [&counter](size_t i) {
        counter.fetch_add(1);
    });
    
    EXPECT_EQ(counter.load(), size);
    
    // Submit additional jobs
    std::vector<JobHandle> handles;
    for (size_t i = 0; i < 100; ++i)
    {
        handles.push_back(jobSystem->submit([&counter]() {
            counter.fetch_add(1);
        }));
    }
    
    for (auto& handle : handles)
    {
        handle.wait();
    }
    
    EXPECT_EQ(counter.load(), size + 100);
}

