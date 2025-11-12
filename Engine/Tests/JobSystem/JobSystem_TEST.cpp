#include <gtest/gtest.h>
#include <Foundation/JobSystem/JobSystem.h>
#include <atomic>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>

using namespace EngineCore::Foundation;

class JobSystemTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        jobSystem = std::make_unique<JobSystem>();
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
// Startup/Shutdown Tests
// ============================================================================

TEST_F(JobSystemTest, StartupShutdown)
{
    jobSystem->startup();
    EXPECT_GT(jobSystem->getWorkerCount(), 0);
    
    jobSystem->shutdown();
    EXPECT_EQ(jobSystem->getWorkerCount(), 0);
}

TEST_F(JobSystemTest, MultipleStartupShutdown)
{
    for (int i = 0; i < 3; ++i)
    {
        jobSystem->startup();
        EXPECT_GT(jobSystem->getWorkerCount(), 0);
        
        jobSystem->shutdown();
        EXPECT_EQ(jobSystem->getWorkerCount(), 0);
    }
}

TEST_F(JobSystemTest, ShutdownWithoutStartup)
{
    // Should not crash
    jobSystem->shutdown();
}

TEST_F(JobSystemTest, DestructorCallsShutdown)
{
    jobSystem->startup();
    EXPECT_GT(jobSystem->getWorkerCount(), 0);
    
    jobSystem.reset();
    // Destructor should call shutdown
}

// ============================================================================
// Basic Job Submission Tests
// ============================================================================

TEST_F(JobSystemTest, SubmitSingleJob)
{
    jobSystem->startup();
    
    std::atomic<bool> executed{false};
    
    auto handle = jobSystem->submit([&executed]() {
        executed = true;
    });
    
    handle.wait();
    
    EXPECT_TRUE(executed.load());
}

TEST_F(JobSystemTest, SubmitJobWithoutStartup)
{
    // Should execute immediately if not started
    std::atomic<bool> executed{false};
    
    auto handle = jobSystem->submit([&executed]() {
        executed = true;
    });
    
    // Job should execute synchronously
    EXPECT_TRUE(executed.load());
    EXPECT_EQ(handle.counter->load(), 0);
}

TEST_F(JobSystemTest, SubmitMultipleJobs)
{
    jobSystem->startup();
    
    const size_t jobCount = 10;
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
        EXPECT_TRUE(executed[i].load()) << "Job " << i << " was not executed";
    }
}

TEST_F(JobSystemTest, SubmitJobWithName)
{
    jobSystem->startup();
    
    std::atomic<bool> executed{false};
    
    auto handle = jobSystem->submit([&executed]() {
        executed = true;
    }, "TestJob");
    
    handle.wait();
    
    EXPECT_TRUE(executed.load());
}

// ============================================================================
// JobHandle Tests
// ============================================================================

TEST_F(JobSystemTest, JobHandleDefaultConstructor)
{
    JobHandle handle;
    EXPECT_EQ(handle.counter->load(), 0);
}

TEST_F(JobSystemTest, JobHandleMoveConstructor)
{
    jobSystem->startup();
    
    // Test move on empty handle
    JobHandle empty1;
    JobHandle empty2(std::move(empty1));
    EXPECT_EQ(empty1.counter->load(), 0);
    EXPECT_EQ(empty2.counter->load(), 0);
    
    // Test move on completed handle
    std::atomic<bool> executed{false};
    
    auto handle1 = jobSystem->submit([&executed]() {
        executed = true;
    });
    
    // Wait for completion first
    handle1.wait();
    EXPECT_TRUE(executed.load());
    EXPECT_EQ(handle1.counter->load(), 0);
    
    // Now move is safe
    JobHandle handle2(std::move(handle1));
    EXPECT_EQ(handle1.counter->load(), 0);
    EXPECT_EQ(handle2.counter->load(), 0);
}

TEST_F(JobSystemTest, JobHandleMoveAssignment)
{
    jobSystem->startup();
    
    // Test move assignment on empty handles
    JobHandle empty1;
    JobHandle empty2;
    empty2 = std::move(empty1);
    EXPECT_EQ(empty1.counter->load(), 0);
    EXPECT_EQ(empty2.counter->load(), 0);
    
    // Test move assignment on completed handle
    std::atomic<bool> executed{false};
    
    auto handle1 = jobSystem->submit([&executed]() {
        executed = true;
    });
    
    // Wait for completion first
    handle1.wait();
    EXPECT_TRUE(executed.load());
    EXPECT_EQ(handle1.counter->load(), 0);
    
    JobHandle handle2;
    handle2 = std::move(handle1);
    
    EXPECT_EQ(handle1.counter->load(), 0);
    EXPECT_EQ(handle2.counter->load(), 0);
}

TEST_F(JobSystemTest, JobHandleSelfMoveAssignment)
{
    jobSystem->startup();
    
    std::atomic<bool> executed{false};
    
    auto handle = jobSystem->submit([&executed]() {
        executed = true;
    });
    
    // Wait for completion first
    handle.wait();
    EXPECT_TRUE(executed.load());
    EXPECT_EQ(handle.counter->load(), 0);
    
    // Self assignment should be safe
    handle = std::move(handle);
    
    EXPECT_EQ(handle.counter->load(), 0);
}

TEST_F(JobSystemTest, JobHandleWait)
{
    jobSystem->startup();
    
    std::atomic<bool> executed{false};
    
    auto handle = jobSystem->submit([&executed]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        executed = true;
    });
    
    EXPECT_FALSE(executed.load());
    handle.wait();
    EXPECT_TRUE(executed.load());
    EXPECT_EQ(handle.counter->load(), 0);
}

// ============================================================================
// Shared Handle Tests
// ============================================================================

TEST_F(JobSystemTest, SubmitWithSharedHandle)
{
    jobSystem->startup();
    
    std::atomic<int> counter{0};
    JobHandle sharedHandle;
    
    const size_t jobCount = 5;
    for (size_t i = 0; i < jobCount; ++i)
    {
        jobSystem->submit([&counter]() {
            counter.fetch_add(1);
        }, sharedHandle);
    }
    
    sharedHandle.wait();
    
    EXPECT_EQ(counter.load(), jobCount);
    EXPECT_EQ(sharedHandle.counter->load(), 0);
}

TEST_F(JobSystemTest, SubmitWithSharedHandleAndName)
{
    jobSystem->startup();
    
    std::atomic<int> counter{0};
    JobHandle sharedHandle;
    
    jobSystem->submit([&counter]() {
        counter.fetch_add(1);
    }, sharedHandle, "SharedJob1");
    
    jobSystem->submit([&counter]() {
        counter.fetch_add(1);
    }, sharedHandle, "SharedJob2");
    
    sharedHandle.wait();
    
    EXPECT_EQ(counter.load(), 2);
}

TEST_F(JobSystemTest, MultipleSharedHandles)
{
    jobSystem->startup();
    
    std::atomic<int> counter1{0};
    std::atomic<int> counter2{0};
    
    JobHandle handle1;
    JobHandle handle2;
    
    jobSystem->submit([&counter1]() {
        counter1.fetch_add(1);
    }, handle1);
    
    jobSystem->submit([&counter2]() {
        counter2.fetch_add(1);
    }, handle2);
    
    handle1.wait();
    handle2.wait();
    
    EXPECT_EQ(counter1.load(), 1);
    EXPECT_EQ(counter2.load(), 1);
}

// ============================================================================
// Parallel Execution Tests
// ============================================================================

TEST_F(JobSystemTest, ParallelExecution)
{
    jobSystem->startup();
    
    const size_t jobCount = 20;
    std::vector<std::atomic<bool>> executed(jobCount);
    std::vector<JobHandle> handles;
    
    for (size_t i = 0; i < jobCount; ++i)
    {
        executed[i] = false;
        handles.push_back(jobSystem->submit([&executed, i]() {
            // Simulate some work
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
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

TEST_F(JobSystemTest, ConcurrentJobSubmission)
{
    jobSystem->startup();
    
    const size_t threadCount = 4;
    const size_t jobsPerThread = 10;
    std::atomic<size_t> totalExecuted{0};
    std::vector<std::thread> threads;
    
    for (size_t t = 0; t < threadCount; ++t)
    {
        threads.emplace_back([this, &totalExecuted, jobsPerThread]() {
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
    
    for (auto& thread : threads)
    {
        thread.join();
    }
    
    EXPECT_EQ(totalExecuted.load(), threadCount * jobsPerThread);
}

// ============================================================================
// Wait Function Tests
// ============================================================================

TEST_F(JobSystemTest, WaitFunction)
{
    jobSystem->startup();
    
    std::atomic<bool> executed{false};
    
    auto handle = jobSystem->submit([&executed]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        executed = true;
    });
    
    jobSystem->wait(handle);
    
    EXPECT_TRUE(executed.load());
    EXPECT_EQ(handle.counter->load(), 0);
}

TEST_F(JobSystemTest, WaitMultipleHandles)
{
    jobSystem->startup();
    
    const size_t jobCount = 5;
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
        jobSystem->wait(handle);
    }
    
    for (size_t i = 0; i < jobCount; ++i)
    {
        EXPECT_TRUE(executed[i].load());
    }
}

// ============================================================================
// Edge Cases
// ============================================================================

TEST_F(JobSystemTest, EmptyJob)
{
    jobSystem->startup();
    
    auto handle = jobSystem->submit([]() {
        // Empty job
    });
    
    handle.wait();
    EXPECT_EQ(handle.counter->load(), 0);
}

TEST_F(JobSystemTest, JobThrowsException)
{
    jobSystem->startup();
    
    std::atomic<bool> executed{false};
    
    auto handle = jobSystem->submit([&executed]() {
        executed = true;
        throw std::runtime_error("Test exception");
    });
    
    // Should not crash, but job completes
    handle.wait();
    EXPECT_TRUE(executed.load());
}

TEST_F(JobSystemTest, LongRunningJob)
{
    jobSystem->startup();
    
    std::atomic<bool> executed{false};
    
    auto handle = jobSystem->submit([&executed]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        executed = true;
    });
    
    handle.wait();
    EXPECT_TRUE(executed.load());
}

TEST_F(JobSystemTest, ManySmallJobs)
{
    jobSystem->startup();
    
    const size_t jobCount = 100;
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

TEST_F(JobSystemTest, JobWithCapture)
{
    jobSystem->startup();
    
    int value = 42;
    std::atomic<int> result{0};
    
    auto handle = jobSystem->submit([value, &result]() {
        result = value * 2;
    });
    
    handle.wait();
    EXPECT_EQ(result.load(), 84);
}

// ============================================================================
// Thread Safety Tests
// ============================================================================

TEST_F(JobSystemTest, ThreadSafeJobSubmission)
{
    jobSystem->startup();
    
    const size_t threadCount = 8;
    const size_t jobsPerThread = 20;
    std::atomic<size_t> totalExecuted{0};
    std::vector<std::thread> threads;
    
    for (size_t t = 0; t < threadCount; ++t)
    {
        threads.emplace_back([this, &totalExecuted, jobsPerThread]() {
            for (size_t i = 0; i < jobsPerThread; ++i)
            {
                auto handle = jobSystem->submit([&totalExecuted]() {
                    totalExecuted.fetch_add(1);
                });
                handle.wait();
            }
        });
    }
    
    for (auto& thread : threads)
    {
        thread.join();
    }
    
    EXPECT_EQ(totalExecuted.load(), threadCount * jobsPerThread);
}

TEST_F(JobSystemTest, ConcurrentSharedHandleSubmission)
{
    jobSystem->startup();
    
    const size_t threadCount = 4;
    const size_t jobsPerThread = 10;
    std::atomic<size_t> totalExecuted{0};
    JobHandle sharedHandle;
    std::vector<std::thread> threads;
    
    for (size_t t = 0; t < threadCount; ++t)
    {
        threads.emplace_back([this, &totalExecuted, &sharedHandle, jobsPerThread]() {
            for (size_t i = 0; i < jobsPerThread; ++i)
            {
                jobSystem->submit([&totalExecuted]() {
                    totalExecuted.fetch_add(1);
                }, sharedHandle);
            }
        });
    }
    
    for (auto& thread : threads)
    {
        thread.join();
    }
    
    sharedHandle.wait();
    EXPECT_EQ(totalExecuted.load(), threadCount * jobsPerThread);
}

