#pragma once
#include <algorithm>
#include <atomic>
#include <functional>
#include <memory>
#include <mutex>
#include <vector>

namespace EngineCore::Foundation
{
template <typename... Args> class Event
{
  public:
    using Handler   = std::function<void(Args...)>;
    using HandlerID = uint64_t;

    class Subscription
    {
        Event* m_event = nullptr;
        HandlerID m_id = 0;

      public:
        Subscription() = default;
        Subscription(Event* ev, HandlerID id) noexcept : m_event(ev), m_id(id)
        {
        }
        Subscription(const Subscription&)            = delete;
        Subscription& operator=(const Subscription&) = delete;
        Subscription(Subscription&& other) noexcept :
            m_event(std::exchange(other.m_event, nullptr)), m_id(std::exchange(other.m_id, 0))
        {
        }
        Subscription& operator=(Subscription&& other) noexcept
        {
            if (this != &other)
            {
                unsubscribe();
                m_event = std::exchange(other.m_event, nullptr);
                m_id    = std::exchange(other.m_id, 0);
            }
            return *this;
        }
        ~Subscription()
        {
            unsubscribe();
        }
        void unsubscribe()
        {
            if (m_event && m_id)
                m_event->unsubscribe(m_id);
            m_event = nullptr;
            m_id    = 0;
        }
    };

  public:
    Event()                        = default;
    ~Event()                       = default;
    Event(const Event&)            = delete;
    Event& operator=(const Event&) = delete;

    /// Подписка (возвращает RAII-объект)
    [[nodiscard]]
    Subscription subscribe(const Handler& handler)
    {
        std::scoped_lock lock(m_mutex);
        HandlerID id = ++m_nextId;
        m_handlers.emplace_back(id, handler);
        return Subscription(this, id);
    }

    /// Отписка по id
    void unsubscribe(HandlerID id)
    {
        std::scoped_lock lock(m_mutex);
        auto it = std::remove_if(m_handlers.begin(), m_handlers.end(), [id](const auto& h) { return h.first == id; });
        m_handlers.erase(it, m_handlers.end());
    }

    /// Вызов события
    void operator()(Args... args)
    {
        std::vector<Handler> snapshot;
        {
            std::scoped_lock lock(m_mutex);
            snapshot.reserve(m_handlers.size());
            for (auto& [_, fn] : m_handlers)
                snapshot.push_back(fn);
        }

        for (auto& fn : snapshot)
        {
            try
            {
                fn(args...);
            }
            catch (const std::exception& e)
            {
                // можно добавить глобальный логгер здесь
            }
        }
    }

    /// Очистить всех слушателей
    void clear()
    {
        std::scoped_lock lock(m_mutex);
        m_handlers.clear();
    }

    /// Проверка, есть ли активные подписчики
    bool empty() const noexcept
    {
        std::scoped_lock lock(m_mutex);
        return m_handlers.empty();
    }

  private:
    mutable std::mutex m_mutex;
    std::vector<std::pair<HandlerID, Handler>> m_handlers;
    std::atomic_uint64_t m_nextId{0};
};
} // namespace EngineCore::Foundation
