#pragma once 

#include <vector>
#include <functional>
#include <mutex>
#include <algorithm>

/// <summary>
/// A thread-safe, generic event system that allows subscribing, unsubscribing, and invoking event handlers.
/// </summary>
/// <typeparam name="Args">Variadic template parameters representing the event's argument types.</typeparam>
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
    /// <summary>
    /// Defines a function type for event handlers.
    /// </summary>
    using Handler = std::function<void(Args...)>;

    /// <summary>
    /// Subscribes a handler to the event.
    /// </summary>
    /// <param name="handler">The function to be called when the event is triggered.</param>
    /// <returns>A unique subscription ID for the handler.</returns>
    int subscribe(const Handler& handler)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        int id = nextId_++;
        m_handlers.push_back({ id, handler });
        return id;
    }

    /// <summary>
    /// Unsubscribes a handler using its subscription ID.
    /// </summary>
    /// <param name="id">The unique subscription ID of the handler to remove.</param>
    void unsubscribe(int id)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto it = std::remove_if(m_handlers.begin(), m_handlers.end(),
            [id](const auto& pair) {
                return pair.first == id;
            });
        m_handlers.erase(it, m_handlers.end());
    }

    /// <summary>
    /// Invokes all subscribed handlers with the provided arguments.
    /// </summary>
    /// <param name="args">Arguments to pass to the subscribed handlers.</param>
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
    std::mutex m_mutex; ///< Mutex for ensuring thread safety.
    int nextId_{ 0 }; ///< Counter for generating unique handler IDs.
    std::vector<std::pair<int, Handler>> m_handlers; ///< List of registered handlers with unique IDs.
};
