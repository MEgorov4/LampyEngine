#pragma once
#include "Core.h"
#include "IModule.h"

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <list>

namespace EngineCore::Base
{
    class ContextLocator final
    {
    public:
        explicit ContextLocator(CoreLocator& core) noexcept
            : m_core(core) {}

        template <typename T>
        void registerMinor(std::shared_ptr<T> module, int priority = 0)
        {
            static_assert(std::is_base_of_v<IModule, T>, "T must inherit from IModule");
            m_minor.emplace_back(std::type_index(typeid(T)), std::move(module), priority);
        }

        template <typename T>
        void registerMajor(std::shared_ptr<T> module, int priority = 0)
        {
            static_assert(std::is_base_of_v<IModule, T>, "T must inherit from IModule");
            m_major.emplace_back(std::type_index(typeid(T)), std::move(module), priority);
        }

        template <typename T>
        T& get()
        {
            const std::type_index id = std::type_index(typeid(T));

            for (auto& m : m_minor)
                if (m.id == id)
                    return *std::static_pointer_cast<T>(m.instance);

            for (auto& m : m_major)
                if (m.id == id)
                    return *std::static_pointer_cast<T>(m.instance);

            return m_core.get<T>();
        }

        void startupMinor()  { startList(m_minor); }
        void startupMajor()  { startList(m_major); }

        void shutdownAll()
        {
            shutdownList(m_major);
            shutdownList(m_minor);
            m_major.clear();
            m_minor.clear();
        }

    private:
        struct Entry
        {
            std::type_index id;
            std::shared_ptr<IModule> instance;
            int priority;

            Entry(std::type_index i, std::shared_ptr<IModule> p, int pr) noexcept
                : id(i), instance(std::move(p)), priority(pr) {}
        };

        CoreLocator& m_core;
        std::list<Entry> m_minor;
        std::list<Entry> m_major;

        static void startList(std::list<Entry>& list)
        {
            list.sort([](const Entry& a, const Entry& b)
            {
                return a.priority < b.priority;
            });

            for (auto& m : list)
                m.instance->startup();
        }

        static void shutdownList(std::list<Entry>& list)
        {
            list.sort([](const Entry& a, const Entry& b)
            {
                return a.priority > b.priority;
            });

            for (auto& m : list)
                m.instance->shutdown();
        }
    };
}
