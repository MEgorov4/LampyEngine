#pragma once

#include <EngineMinimal.h>
#include <SDL3/SDL.h>

namespace TimeModule
{
    /// <summary>
    /// Вспомогательный класс для ручного измерения времени между событиями.
    /// Используется для профилирования без Tracy.
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
        /// Сохраняет текущее значение счётчика времени.
        /// </summary>
        void start() noexcept
        {
            m_startCounter = SDL_GetPerformanceCounter();
            m_started = true;
        }

        /// <summary>
        /// Возвращает прошедшее время с момента start() в секундах.
        /// </summary>
        /// <returns>Время в секундах, или 0.0f если start() не был вызван.</returns>
        float stop() noexcept
        {
            if (!m_started || m_freq == 0)
                return 0.0f;

            Uint64 currentCounter = SDL_GetPerformanceCounter();
            Uint64 elapsed = currentCounter - m_startCounter;
            return static_cast<float>(static_cast<double>(elapsed) / static_cast<double>(m_freq));
        }

        /// <summary>
        /// Возвращает прошедшее время в миллисекундах.
        /// </summary>
        /// <returns>Время в миллисекундах, или 0.0f если start() не был вызван.</returns>
        float getElapsedMs() const noexcept
        {
            if (!m_started || m_freq == 0)
                return 0.0f;

            Uint64 currentCounter = SDL_GetPerformanceCounter();
            Uint64 elapsed = currentCounter - m_startCounter;
            return static_cast<float>(static_cast<double>(elapsed) * 1000.0 / static_cast<double>(m_freq));
        }

        /// <summary>
        /// Проверяет, прошло ли заданное количество секунд с момента start().
        /// </summary>
        /// <param name="seconds">Количество секунд для проверки.</param>
        /// <returns>true если прошло указанное время, false иначе.</returns>
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
        /// Сбрасывает состояние таймера.
        /// </summary>
        void reset() noexcept
        {
            m_started = false;
            m_startCounter = 0;
        }
    };
}

