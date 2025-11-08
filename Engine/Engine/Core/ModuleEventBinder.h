#pragma once
#include "../Foundation/Event/EventBus.h"
#include "EventHelpers.h"
#include "Foundation/Profiler/ProfileAllocator.h"
namespace EngineCore::Base
{

class ModuleEventBinder
{
    std::vector<std::shared_ptr<void>> m_subscriptions;

  public:
    ModuleEventBinder() = default;
    ~ModuleEventBinder()
    {
        clear();
    }

    ModuleEventBinder(const ModuleEventBinder &) = delete;
    ModuleEventBinder &operator=(const ModuleEventBinder &) = delete;

    ModuleEventBinder(ModuleEventBinder &&) noexcept = default;
    ModuleEventBinder &operator=(ModuleEventBinder &&) noexcept = default;

    template <typename TEvent, typename TClass> void bind(TClass *instance, void (TClass::*method)(const TEvent &))
    {
        auto sub = GCEB().subscribe<TEvent>([instance, method](const TEvent &e) { (instance->*method)(e); });
        m_subscriptions.push_back(std::make_shared<decltype(sub)>(std::move(sub)));
    }

    template <typename TEvent, typename Fn> void bind(Fn &&func)
    {
        auto sub = GCEB().subscribe<TEvent>(std::forward<Fn>(func));
        m_subscriptions.push_back(std::make_shared<decltype(sub)>(std::move(sub)));
    }

    template <typename TEvent, typename Fn> void bindOnce(Fn &&func)
    {
        using SubscriptionT = EngineCore::Foundation::EventBus::Subscription<TEvent>;
        auto subPtr = std::make_shared<SubscriptionT>();

        *subPtr = GCEB().subscribe<TEvent>([subPtr, func = std::forward<Fn>(func)](const TEvent &e) mutable {
            func(e);
            subPtr->unsubscribe(); // автоотписка
        });

        m_subscriptions.push_back(subPtr);
    }

    /// Очистить все подписки
    void clear()
    {
        m_subscriptions.clear();
    }
};
} // namespace EngineCore::Base
