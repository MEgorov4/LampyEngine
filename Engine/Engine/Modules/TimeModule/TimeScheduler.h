#pragma once

#include <EngineMinimal.h>
#include <functional>
#include <vector>
#include <mutex>

namespace TimeModule
{
    /// <summary>
    /// </summary>
    class TimeScheduler
    {
    private:
        struct Task
        {
            double nextInvoke;
            double interval;
            bool repeating;
            std::function<void()> fn;

            Task(double nextInvoke, double interval, bool repeating, std::function<void()> fn)
                : nextInvoke(nextInvoke), interval(interval), repeating(repeating), fn(std::move(fn))
            {
            }
        };

        std::vector<Task> m_tasks;
        mutable std::mutex m_mutex;

    public:
        /// <summary>
        /// </summary>
        void schedule(std::function<void()> fn, double delaySec) noexcept
        {
            if (delaySec < 0.0)
                delaySec = 0.0;

            std::lock_guard<std::mutex> lock(m_mutex);
            m_tasks.emplace_back(0.0, delaySec, false, std::move(fn));
        }

        /// <summary>
        /// </summary>
        void scheduleRepeating(std::function<void()> fn, double intervalSec) noexcept
        {
            if (intervalSec < 0.0)
                intervalSec = 0.0;

            std::lock_guard<std::mutex> lock(m_mutex);
            m_tasks.emplace_back(0.0, intervalSec, true, std::move(fn));
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

