#include "JobSystem.h"

#include <EngineMinimal.h>
#include "../Log/LoggerMacro.h"
#include "../Log/LogVerbosity.h"
#include <format>

#ifdef TRACY_ENABLE
#include <tracy/Tracy.hpp>
#endif

using namespace EngineCore::Foundation;

JobSystem::~JobSystem()
{
    shutdown();
}

void JobSystem::startup()
{
#ifdef TRACY_ENABLE
    ZoneScopedN("JobSystem::startup");
#endif

    LT_LOGI("JobSystem", "Initializing Job System...");

    m_running = true;
    const size_t threadCount = std::max<size_t>(1, std::thread::hardware_concurrency() - 1);
    m_workers.clear();
    m_workers.reserve(threadCount);
    for (size_t i = 0; i < threadCount; ++i)
    {
        m_workers.emplace_back();
    }

    for (size_t i = 0; i < threadCount; ++i)
    {
        m_workers[i].thread = std::thread([this, i]() {
#ifdef TRACY_ENABLE
            // Name worker thread in Tracy
            char name[64];
            std::snprintf(name, sizeof(name), "JobWorker %zu", i);
            tracy::SetThreadName(name);
#endif
            workerLoop(i);
        });
    }

    LT_LOGI("JobSystem", std::format("Started with {} worker threads", threadCount));
}

void JobSystem::shutdown()
{
#ifdef TRACY_ENABLE
    ZoneScopedN("JobSystem::shutdown");
#endif

    if (!m_running)
        return;

    {
        std::lock_guard lock(m_cvMutex);
        m_running = false;
    }

    m_cv.notify_all();

    for (auto& w : m_workers)
    {
        if (w.thread.joinable())
            w.thread.join();
    }

    LT_LOGI("JobSystem", "Shutdown complete");
}

JobHandle JobSystem::submit(std::function<void()> job)
{
    JobHandle handle;
    submit(std::move(job), handle);
    return std::move(handle);  // Explicit move to avoid copy attempt
}

void JobSystem::submit(std::function<void()> job, JobHandle& handle)
{
    if (m_workers.empty())
    {
        // If not started, execute immediately
        job();
        return;
    }

    handle.counter.fetch_add(1, std::memory_order_relaxed);

    // Round-robin distribution
    static std::atomic<size_t> next{0};
    size_t idx = next.fetch_add(1) % m_workers.size();

    {
        std::lock_guard lock(m_workers[idx].queueMutex);
        m_workers[idx].queue.emplace_back([job = std::move(job), &handle]() {
#ifdef TRACY_ENABLE
            ZoneScopedN("JobExecute");
#endif
            job();

            handle.counter.fetch_sub(1, std::memory_order_release);
        });
    }

    m_cv.notify_one();
}

void JobSystem::wait(const JobHandle& handle)
{
    handle.wait();
}

void JobSystem::workerLoop(size_t index)
{
#ifdef TRACY_ENABLE
    ZoneScopedN("JobSystem::workerLoop");
#endif

    while (true)
    {
        std::function<void()> job;

        {
            std::unique_lock lock(m_workers[index].queueMutex);

            if (!m_workers[index].queue.empty())
            {
                job = std::move(m_workers[index].queue.front());
                m_workers[index].queue.pop_front();
            }
            else if (!stealJob(index, job))
            {
                m_cv.wait_for(lock, std::chrono::milliseconds(2));

                if (!m_running && m_workers[index].queue.empty())
                    return;

                continue;
            }
        }

        if (job)
        {
#ifdef TRACY_ENABLE
            ZoneScopedN("JobSystem::Invoke");
#endif
            job();
        }
    }
}

bool JobSystem::stealJob(size_t thiefIndex, std::function<void()>& job)
{
    std::mt19937 rng(static_cast<unsigned int>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count() + thiefIndex));
    std::uniform_int_distribution<size_t> dist(0, m_workers.size() - 1);

    for (size_t attempt = 0; attempt < m_workers.size(); ++attempt)
    {
        size_t victim = dist(rng);

        if (victim == thiefIndex)
            continue;

        auto& q = m_workers[victim];

        std::lock_guard lock(q.queueMutex);

        if (!q.queue.empty())
        {
            job = std::move(q.queue.back());
            q.queue.pop_back();
            return true;
        }
    }

    return false;
}
