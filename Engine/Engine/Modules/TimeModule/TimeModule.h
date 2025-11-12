#pragma once

#include <EngineMinimal.h>
#include <SDL3/SDL.h>
#include "TimeTimer.h"
#include "TimeScheduler.h"

namespace TimeModule
{
    /// <summary>
    /// </summary>
    class TimeModule : public IModule
    {
    private:
        Uint64 m_prevCounter = 0;
        Uint64 m_freq = 0;
        float m_deltaTime = 0.0f;
        double m_elapsedSeconds = 0.0;
        float m_timeScale = 1.0f;
        bool m_initialized = false;

        TimeScheduler m_scheduler;

        struct Scheduled
        {
            double invokeTime;
            std::function<void()> fn;

            Scheduled(double invokeTime, std::function<void()> fn)
                : invokeTime(invokeTime), fn(std::move(fn))
            {
            }
        };

        std::vector<Scheduled> m_scheduled;

    public:
        TimeModule() = default;
        ~TimeModule() override = default;

        /// <summary>
        /// </summary>
        void startup() override;

        /// <summary>
        /// </summary>
        void shutdown() override;

        /// <summary>
        /// </summary>
        void tick(float deltaTime = 0.0f) noexcept;

        /// <summary>
        /// </summary>
        float getDeltaTime() const noexcept
        {
            return m_deltaTime;
        }

        /// <summary>
        /// </summary>
        double getElapsedSeconds() const noexcept
        {
            return m_elapsedSeconds;
        }

        /// <summary>
        /// </summary>
        void setTimeScale(float scale) noexcept
        {
            m_timeScale = std::max(0.0f, scale);
        }

        /// <summary>
        /// </summary>
        float getTimeScale() const noexcept
        {
            return m_timeScale;
        }

        /// <summary>
        /// </summary>
        void sleepForMs(uint32_t ms) const noexcept
        {
            SDL_Delay(ms);
        }

        /// <summary>
        /// </summary>
        void schedule(std::function<void()> fn, double delaySec) noexcept
        {
            if (delaySec < 0.0)
                delaySec = 0.0;

            double invokeTime = m_elapsedSeconds + delaySec;
            m_scheduled.emplace_back(invokeTime, std::move(fn));
        }

        /// <summary>
        /// </summary>
        TimeScheduler& getScheduler() noexcept
        {
            return m_scheduler;
        }

        /// <summary>
        /// </summary>
        const TimeScheduler& getScheduler() const noexcept
        {
            return m_scheduler;
        }

        /// <summary>
        /// </summary>
        static TimeTimer createTimer() noexcept
        {
            return TimeTimer();
        }
    };
}

