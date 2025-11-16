#pragma once
#include "Foundation/Profiler/ProfileAllocator.h"

#include <functional>
#include <memory>
#include <mutex>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace EngineCore::Foundation
{

class EventBus
{
  public:
    template <typename T> using Handler = std::function<void(const T&)>;

    template <typename T> class Subscription
    {
        EventBus* m_bus = nullptr;
        size_t m_id     = 0;
        std::weak_ptr<bool> m_busToken;

      public:
        Subscription() = default;
        Subscription(EventBus* bus, size_t id, std::shared_ptr<bool> token) noexcept :
            m_bus(bus), m_id(id), m_busToken(std::move(token))
        {
        }
        Subscription(Subscription&& other) noexcept :
            m_bus(std::exchange(other.m_bus, nullptr)),
            m_id(std::exchange(other.m_id, 0)),
            m_busToken(std::move(other.m_busToken))
        {
        }
        Subscription& operator=(Subscription&& other) noexcept
        {
            if (this != &other)
            {
                unsubscribe();
                m_bus      = std::exchange(other.m_bus, nullptr);
                m_id       = std::exchange(other.m_id, 0);
                m_busToken = std::move(other.m_busToken);
            }
            return *this;
        }
        ~Subscription()
        {
            unsubscribe();
        }

        void unsubscribe()
        {
            if (m_bus && m_id)
            {
                if (auto token = m_busToken.lock())
                {
                    if (*token)
                    {
                        m_bus->unsubscribe<T>(m_id);
                    }
                }
            }
            m_bus = nullptr;
            m_id  = 0;
            m_busToken.reset();
        }
    };

  public:
    EventBus()  = default;
    ~EventBus()
    {
        if (m_aliveToken)
            *m_aliveToken = false;
        clear();
    }

    template <typename T>
    [[nodiscard]]
    Subscription<T> subscribe(Handler<T> handler)
    {
        std::scoped_lock lock(m_mutex);
        auto& list = m_handlers[typeid(T)];
        size_t id  = ++m_nextId;
        list.push_back(std::make_shared<Wrapper<T>>(id, std::move(handler)));
        return Subscription<T>(this, id, m_aliveToken);
    }

    template <typename T> void unsubscribe(size_t id)
    {
        std::scoped_lock lock(m_mutex);
        auto it = m_handlers.find(typeid(T));
        if (it == m_handlers.end())
            return;
        auto& list = it->second;
        list.erase(std::remove_if(list.begin(), list.end(), [id](auto& ptr) { return ptr->id == id; }), list.end());
    }

    template <typename T> void emit(const T& event)
    {
        std::vector<std::shared_ptr<BaseWrapper>, ProfileAllocator<std::shared_ptr<BaseWrapper>>> snapshot;
        {
            std::scoped_lock lock(m_mutex);
            auto it = m_handlers.find(typeid(T));
            if (it == m_handlers.end())
                return;
            snapshot = it->second;
        }
        for (auto& ptr : snapshot)
            static_cast<Wrapper<T>*>(ptr.get())->fn(event);
    }

    void clear()
    {
        std::scoped_lock lock(m_mutex);
        m_handlers.clear();
    }

  private:
    struct BaseWrapper
    {
        size_t id;
        virtual ~BaseWrapper() = default;
    };
    template <typename T> struct Wrapper : BaseWrapper
    {
        Handler<T> fn;
        Wrapper(size_t i, Handler<T> f)
        {
            id = i;
            fn = std::move(f);
        }
    };

    mutable std::mutex m_mutex;
    std::unordered_map<std::type_index,
                       std::vector<std::shared_ptr<BaseWrapper>, ProfileAllocator<std::shared_ptr<BaseWrapper>>>>
        m_handlers;
    size_t m_nextId = 0;
    std::shared_ptr<bool> m_aliveToken = std::make_shared<bool>(true);
};

} // namespace EngineCore::Foundation
