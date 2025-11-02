#pragma once

#include <EngineMinimal.h>
#include <functional>
#include <vector>
#include <mutex>

namespace TimeModule
{
    /// <summary>
    /// Система периодических или одноразовых событий.
    /// Может использоваться для событий, анимаций, геймплейных задержек и т.д.
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
        /// Запланировать одноразовое событие.
        /// </summary>
        /// <param name="fn">Функция для вызова.</param>
        /// <param name="delaySec">Задержка в секундах.</param>
        void schedule(std::function<void()> fn, double delaySec) noexcept
        {
            if (delaySec < 0.0)
                delaySec = 0.0;

            std::lock_guard<std::mutex> lock(m_mutex);
            // nextInvoke будет установлен в update() с учетом текущего времени
            m_tasks.emplace_back(0.0, delaySec, false, std::move(fn));
        }

        /// <summary>
        /// Запланировать повторяющееся событие.
        /// </summary>
        /// <param name="fn">Функция для вызова.</param>
        /// <param name="intervalSec">Интервал повторения в секундах.</param>
        void scheduleRepeating(std::function<void()> fn, double intervalSec) noexcept
        {
            if (intervalSec < 0.0)
                intervalSec = 0.0;

            std::lock_guard<std::mutex> lock(m_mutex);
            // nextInvoke будет установлен в update() с учетом текущего времени
            m_tasks.emplace_back(0.0, intervalSec, true, std::move(fn));
        }

        /// <summary>
        /// Отменить все запланированные задачи.
        /// </summary>
        void cancelAll() noexcept
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_tasks.clear();
        }

        /// <summary>
        /// Проверить и вызвать готовые задачи.
        /// </summary>
        /// <param name="currentTime">Текущее время в секундах.</param>
        void update(double currentTime) noexcept
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            for (size_t i = 0; i < m_tasks.size();)
            {
                Task& task = m_tasks[i];

                // Если nextInvoke еще не установлен (только что добавлена), устанавливаем его
                if (task.nextInvoke == 0.0)
                {
                    task.nextInvoke = currentTime + task.interval;
                }

                if (currentTime >= task.nextInvoke)
                {
                    // Вызываем функцию
                    try
                    {
                        task.fn();
                    }
                    catch (...)
                    {
                        // Игнорируем исключения в колбэках
                    }

                    if (task.repeating)
                    {
                        // Обновляем время следующего вызова
                        task.nextInvoke = currentTime + task.interval;
                        ++i;
                    }
                    else
                    {
                        // Удаляем одноразовую задачу
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
        /// Получить количество активных задач.
        /// </summary>
        /// <returns>Количество активных задач.</returns>
        size_t getTaskCount() const noexcept
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            return m_tasks.size();
        }
    };
}

