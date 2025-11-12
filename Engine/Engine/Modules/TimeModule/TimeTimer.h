#pragma once

#include <EngineMinimal.h>
#include <SDL3/SDL.h>

namespace TimeModule
{
    /// <summary>
    /// </summary>
    class TimeTimer
    {
        Uint64 m_startCounter = 0;
        Uint64 m_freq = 0;
        bool m_started = false;

    public:
        TimeTimer() noexcept
        {
            m_freq = SDL_GetPerformanceFrequency();
        }

        /// <summary>
        /// </summary>
        void start() noexcept
        {
            m_startCounter = SDL_GetPerformanceCounter();
            m_started = true;
        }

        /// <summary>
        /// </summary>
        float stop() noexcept
        {
            if (!m_started || m_freq == 0)
                return 0.0f;

            Uint64 currentCounter = SDL_GetPerformanceCounter();
            Uint64 elapsed = currentCounter - m_startCounter;
            return static_cast<float>(static_cast<double>(elapsed) / static_cast<double>(m_freq));
        }

        /// <summary>
        /// </summary>
        float getElapsedMs() const noexcept
        {
            if (!m_started || m_freq == 0)
                return 0.0f;

            Uint64 currentCounter = SDL_GetPerformanceCounter();
            Uint64 elapsed = currentCounter - m_startCounter;
            return static_cast<float>(static_cast<double>(elapsed) * 1000.0 / static_cast<double>(m_freq));
        }

        /// <summary>
        /// </summary>
        bool hasPassed(float seconds) const noexcept
        {
            if (!m_started || m_freq == 0)
                return false;

            Uint64 currentCounter = SDL_GetPerformanceCounter();
            Uint64 elapsed = currentCounter - m_startCounter;
            double elapsedSeconds = static_cast<double>(elapsed) / static_cast<double>(m_freq);
            return elapsedSeconds >= static_cast<double>(seconds);
        }

        /// <summary>
        /// </summary>
        void reset() noexcept
        {
            m_started = false;
            m_startCounter = 0;
        }
    };
}

