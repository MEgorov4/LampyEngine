#pragma once

#include <EngineMinimal.h>
#include <functional>
#include <vector>
#include <mutex>
#include <cstddef>

namespace TimeModule
{
    /// <summary>
    /// </summary>
class TimeScheduler
{
public:
        using TaskId = std::size_t;
        static constexpr TaskId InvalidTaskId = 0;

private:
        struct Task
        {
            double nextInvoke;
            double interval;
            bool repeating;
            bool cancelled;
            TaskId id;
            std::function<void()> fn;

            Task(double nextInvoke, double interval, bool repeating, TaskId id, std::function<void()> fn)
                : nextInvoke(nextInvoke)
                , interval(interval)
                , repeating(repeating)
                , cancelled(false)
                , id(id)
                , fn(std::move(fn))
            {
            }
        };

        std::vector<Task> m_tasks;
        mutable std::mutex m_mutex;
        TaskId m_nextTaskId = 1;

    public:
        /// <summary>
        /// </summary>
        TaskId schedule(std::function<void()> fn, double delaySec) noexcept
        {
            if (delaySec < 0.0)
                delaySec = 0.0;

            std::lock_guard<std::mutex> lock(m_mutex);
            TaskId id = m_nextTaskId++;
            m_tasks.emplace_back(0.0, delaySec, false, id, std::move(fn));
            return id;
        }

        /// <summary>
        /// </summary>
        TaskId scheduleRepeating(std::function<void()> fn, double intervalSec) noexcept
        {
            if (intervalSec < 0.0)
                intervalSec = 0.0;

            std::lock_guard<std::mutex> lock(m_mutex);
            TaskId id = m_nextTaskId++;
            m_tasks.emplace_back(0.0, intervalSec, true, id, std::move(fn));
            return id;
        }

        /// <summary>
        /// </summary>
        void cancel(TaskId id) noexcept
        {
            if (id == InvalidTaskId)
                return;

            std::lock_guard<std::mutex> lock(m_mutex);
            for (auto& task : m_tasks)
            {
                if (task.id == id)
                    task.cancelled = true;
            }
        }

        /// <summary>
        /// </summary>
        void cancelAll() noexcept
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_tasks.clear();
        }

        /// <summary>
        /// </summary>
        void update(double currentTime) noexcept
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            for (size_t i = 0; i < m_tasks.size();)
            {
                Task& task = m_tasks[i];

                if (task.cancelled)
                {
                    m_tasks.erase(m_tasks.begin() + static_cast<std::ptrdiff_t>(i));
                    continue;
                }

                if (task.nextInvoke == 0.0)
                {
                    task.nextInvoke = currentTime + task.interval;
                }

                if (currentTime >= task.nextInvoke)
                {
                    try
                    {
                        task.fn();
                    }
                    catch (...)
                    {
                    }

                    if (task.repeating)
                    {
                        task.nextInvoke = currentTime + task.interval;
                        ++i;
                    }
                    else
                    {
                        m_tasks.erase(m_tasks.begin() + static_cast<std::ptrdiff_t>(i));
                    }
                }
                else
                {
                    ++i;
                }
            }
        }

        /// <summary>
        /// </summary>
        size_t getTaskCount() const noexcept
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_tasks.size();
        }
    };
}

