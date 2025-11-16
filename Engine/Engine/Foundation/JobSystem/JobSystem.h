#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <deque>
#include <vector>
#include <functional>
#include <atomic>
#include <memory>
#include <random>
#include <chrono>

#include "../../Core/IModule.h"
// Don't include EngineMinimal.h here to avoid circular dependency
// (Foundation.h includes JobSystem.h)

#ifndef ENGINE_DISABLE_JOB_SYSTEM
#define ENGINE_DISABLE_JOB_SYSTEM 1
#endif

namespace EngineCore::Foundation
{
    struct JobHandle
    {
        std::shared_ptr<std::atomic<uint32_t>> counter;
        
        // Default constructor - explicitly initialize counter
        JobHandle() noexcept : counter(std::make_shared<std::atomic<uint32_t>>(0)) {}
        
        // Delete copy constructor/assignment (handles should not be copied)
        JobHandle(const JobHandle&) = delete;
        JobHandle& operator=(const JobHandle&) = delete;
        
        // Allow move (share the counter)
        JobHandle(JobHandle&& other) noexcept 
            : counter(std::move(other.counter))
        {
            if (!counter)
            {
                counter = std::make_shared<std::atomic<uint32_t>>(0);
            }
            // Ensure moved-from handle has valid counter with zero value
            other.counter = std::make_shared<std::atomic<uint32_t>>(0);
        }
        
        JobHandle& operator=(JobHandle&& other) noexcept
        {
            if (this != &other)
            {
                counter = std::move(other.counter);
                if (!counter)
                {
                    counter = std::make_shared<std::atomic<uint32_t>>(0);
                }
                // Ensure moved-from handle has valid counter with zero value
                other.counter = std::make_shared<std::atomic<uint32_t>>(0);
            }
            return *this;
        }
        
        void wait() const noexcept
        {
            if (!counter)
                return;
            // Active wait (workers continue stealing)
            while (counter->load(std::memory_order_acquire) > 0)
            {
                std::this_thread::yield();
            }
        }
    };

    class JobSystem final : public EngineCore::Base::IModule
    {
    public:
        JobSystem() = default;
        ~JobSystem() override;

        static constexpr bool kJobSystemDisabled = ENGINE_DISABLE_JOB_SYSTEM != 0;

        void startup() override;
        void shutdown() override;

        // Submit single job
        JobHandle submit(std::function<void()> job);
        
        // Submit single job with name for profiling
        JobHandle submit(std::function<void()> job, const char* jobName);

        // Submit with shared handle (to compose parallel tasks)
        void submit(std::function<void()> job, JobHandle& handle);
        
        // Submit with shared handle and name for profiling
        void submit(std::function<void()> job, JobHandle& handle, const char* jobName);

        // Wait for completion
        void wait(const JobHandle& handle);

        // Parallel for (range-based)
        template<typename Fn>
        void parallel_for(size_t begin, size_t end, Fn&& fn, size_t grain = 64);

        // For debugging
        size_t getWorkerCount() const noexcept { return m_workers.size(); }

    private:
        struct Worker
        {
            std::thread thread{};
            std::deque<std::function<void()>> queue{};
            std::mutex queueMutex{};
            
            // Default constructor
            Worker() = default;
            
            // Delete copy (workers should not be copied)
            Worker(const Worker&) = delete;
            Worker& operator=(const Worker&) = delete;
            
            // Allow move (explicitly move thread)
            Worker(Worker&& other) noexcept
                : thread(std::move(other.thread))
                , queue(std::move(other.queue))
                , queueMutex()
            {
                // Mutex cannot be moved, but that's fine - we only move before threads start
            }
            
            Worker& operator=(Worker&& other) noexcept
            {
                if (this != &other)
                {
                    thread = std::move(other.thread);
                    queue = std::move(other.queue);
                    // Mutex cannot be moved, but that's fine
                }
                return *this;
            }
        };

        std::vector<Worker> m_workers;
        std::atomic<bool> m_running{false};
        std::condition_variable m_cv;
        std::mutex m_cvMutex;

        void workerLoop(size_t index);
        bool stealJob(size_t thiefIndex, std::function<void()>& job);
    };

    // Implementation of parallel_for (in header for templates)
    template<typename Fn>
    inline void JobSystem::parallel_for(size_t begin, size_t end, Fn&& fn, size_t grain)
    {
        if (begin >= end)
            return;

        JobHandle handle{};

        size_t total = end - begin;
        size_t workerCount = getWorkerCount();
        if (workerCount == 0)
        {
            // If not started yet, run sequentially
            for (size_t i = begin; i < end; ++i)
                fn(i);
            return;
        }

        size_t chunk = std::max<size_t>(1, total / (workerCount * 2));
        chunk = std::max(chunk, grain);

        for (size_t i = begin; i < end; i += chunk)
        {
            size_t start = i;
            size_t finish = std::min(i + chunk, end);

            submit([start, finish, fn = std::forward<Fn>(fn)]() {
                for (size_t j = start; j < finish; ++j)
                    fn(j);
            }, handle);
        }

        wait(handle);
    }
}
