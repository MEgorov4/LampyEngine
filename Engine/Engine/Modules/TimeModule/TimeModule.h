#pragma once

#include <EngineMinimal.h>
#include <SDL3/SDL.h>
#include "TimeTimer.h"
#include "TimeScheduler.h"

namespace TimeModule
{
    /// <summary>
    /// Главный модуль для управления временем в движке.
    /// Отвечает за измерение delta time, elapsed time, time scale и планирование событий.
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
        /// Инициализация модуля времени.
        /// Запрашивает частоту таймера и сохраняет начальное значение счётчика.
        /// </summary>
        void startup() override;

        /// <summary>
        /// Очистка внутренних коллекций (таймеров, колбэков).
        /// </summary>
        void shutdown() override;

        /// <summary>
        /// Обновление дельта-времени и вызов запланированных событий.
        /// Если deltaTime равен 0, вычисляется автоматически через SDL.
        /// </summary>
        /// <param name="deltaTime">Дельта времени из внешнего источника (опционально, 0 для автовычисления).</param>
        void tick(float deltaTime = 0.0f) noexcept;

        /// <summary>
        /// Возвращает время между кадрами (в секундах).
        /// </summary>
        /// <returns>Дельта времени в секундах.</returns>
        float getDeltaTime() const noexcept
        {
            return m_deltaTime;
        }

        /// <summary>
        /// Возвращает общее время работы движка с момента запуска (в секундах).
        /// </summary>
        /// <returns>Общее время в секундах.</returns>
        double getElapsedSeconds() const noexcept
        {
            return m_elapsedSeconds;
        }

        /// <summary>
        /// Устанавливает множитель времени (1.0f — нормальная скорость).
        /// </summary>
        /// <param name="scale">Множитель времени (1.0f = норма, < 1.0f = замедление, > 1.0f = ускорение).</param>
        void setTimeScale(float scale) noexcept
        {
            m_timeScale = std::max(0.0f, scale);
        }

        /// <summary>
        /// Возвращает текущий множитель времени.
        /// </summary>
        /// <returns>Множитель времени.</returns>
        float getTimeScale() const noexcept
        {
            return m_timeScale;
        }

        /// <summary>
        /// Блокирует выполнение на заданное количество миллисекунд.
        /// </summary>
        /// <param name="ms">Количество миллисекунд.</param>
        void sleepForMs(uint32_t ms) const noexcept
        {
            SDL_Delay(ms);
        }

        /// <summary>
        /// Планирует отложенный вызов функции.
        /// </summary>
        /// <param name="fn">Функция для вызова.</param>
        /// <param name="delaySec">Задержка в секундах.</param>
        void schedule(std::function<void()> fn, double delaySec) noexcept
        {
            if (delaySec < 0.0)
                delaySec = 0.0;

            double invokeTime = m_elapsedSeconds + delaySec;
            m_scheduled.emplace_back(invokeTime, std::move(fn));
        }

        /// <summary>
        /// Получить ссылку на планировщик для более сложных операций.
        /// </summary>
        /// <returns>Ссылка на TimeScheduler.</returns>
        TimeScheduler& getScheduler() noexcept
        {
            return m_scheduler;
        }

        /// <summary>
        /// Получить константную ссылку на планировщик.
        /// </summary>
        /// <returns>Константная ссылка на TimeScheduler.</returns>
        const TimeScheduler& getScheduler() const noexcept
        {
            return m_scheduler;
        }

        /// <summary>
        /// Создать экземпляр TimeTimer.
        /// </summary>
        /// <returns>Новый экземпляр TimeTimer.</returns>
        static TimeTimer createTimer() noexcept
        {
            return TimeTimer();
        }
    };
}

