#include "TimeModule.h"

#include <Foundation/Log/LoggerMacro.h>

namespace TimeModule
{
    void TimeModule::startup()
    {
        m_freq = SDL_GetPerformanceFrequency();
        m_prevCounter = SDL_GetPerformanceCounter();
        m_deltaTime = 0.0f;
        m_elapsedSeconds = 0.0;
        m_timeScale = 1.0f;
        m_initialized = true;

        LT_LOGI("TimeModule", "Startup: Performance frequency = " + std::format("{}", m_freq));
    }

    void TimeModule::shutdown()
    {
        m_scheduled.clear();
        m_scheduler.cancelAll();
        m_initialized = false;

        LT_LOGI("TimeModule", "Shutdown");
    }

    void TimeModule::tick(float deltaTime) noexcept
    {
        if (!m_initialized || m_freq == 0)
            return;

        if (deltaTime <= 0.0f)
        {
            Uint64 currentCounter = SDL_GetPerformanceCounter();
            Uint64 elapsed = currentCounter - m_prevCounter;

            double rawDeltaTime = static_cast<double>(elapsed) / static_cast<double>(m_freq);

            rawDeltaTime = std::clamp(rawDeltaTime, 1e-6, 0.25);

            m_deltaTime = static_cast<float>(rawDeltaTime);
            m_prevCounter = currentCounter;
        }
        else
        {
            m_deltaTime = deltaTime;
        }

        float scaledDeltaTime = m_deltaTime * m_timeScale;

        m_elapsedSeconds += static_cast<double>(scaledDeltaTime);

        m_scheduler.update(m_elapsedSeconds);

        for (size_t i = 0; i < m_scheduled.size();)
        {
            if (m_elapsedSeconds >= m_scheduled[i].invokeTime)
            {
                try
                {
                    m_scheduled[i].fn();
                }
                catch (...)
                {
                }

                m_scheduled.erase(m_scheduled.begin() + static_cast<std::ptrdiff_t>(i));
            }
            else
            {
                ++i;
            }
        }
    }
}

