#pragma once 

#include <vector>
#include <functional>
#include <mutex>
#include <algorithm>

template <typename... Args>
class Event
{
public:
    Event(const Event&) = delete;
    Event& operator=(const Event&) = delete;

    Event(Event&&) = delete;
    Event& operator=(Event&&) = delete;

    Event() = default;
    ~Event() = default;
public:
    using Handler = std::function<void(Args...)>;

    int subscribe(const Handler& handler)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        int id = nextId_++;
        m_handlers.push_back({ id, handler });
        return id;
    }

    void unsubscribe(int id)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = std::remove_if(m_handlers.begin(), m_handlers.end(),
            [id](const auto& pair) {
                return pair.first == id;
            });
        m_handlers.erase(it, m_handlers.end());
    }

    void operator()(Args... args)
    {
        std::vector<Handler> safeCopy;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            for (auto& pair : m_handlers) {
                safeCopy.push_back(pair.second);
            }
        }

        for (auto& handler : safeCopy) {
            handler(args...);
        }
    }

private:
    std::mutex m_mutex;
    int nextId_{ 0 };

    std::vector<std::pair<int, Handler>> m_handlers;
};