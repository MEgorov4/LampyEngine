#pragma once
#include <memory>
#include <typeindex>
#include <stdexcept>
#include <list>
#include "IModule.h"

namespace EngineCore::Base
{
    class CoreLocator final
    {
    public:
        CoreLocator() = default;
        ~CoreLocator() { shutdownAll(); }

        template<typename T>
        void registerModule(std::shared_ptr<T> module, int priority = 0)
        {
            static_assert(std::is_base_of_v<IModule, T>, "T must inherit from IModule");
            m_modules.emplace_back(std::type_index(typeid(T)), std::move(module), priority);
        }

        template<typename T>
        T& get()
        {
            for (auto& m : m_modules)
                if (m.id == std::type_index(typeid(T)))
                    return *std::static_pointer_cast<T>(m.instance);
            throw std::runtime_error("CoreLocator::get() module not found");
        }

        template<typename T>
        std::shared_ptr<T> tryGet() noexcept
        {
            for (auto& m : m_modules)
                if (m.id == std::type_index(typeid(T)))
                    return std::static_pointer_cast<T>(m.instance);
            return nullptr;
        }

        void startupAll()
        {
            // list::sort Ч встроенный метод, безопасен и не инвалидирует ссылки
            m_modules.sort([](const ModuleEntry& a, const ModuleEntry& b)
            {
                return a.priority < b.priority;
            });

            for (auto& m : m_modules)
                m.instance->startup();
        }

        void shutdownAll()
        {
            m_modules.sort([](const ModuleEntry& a, const ModuleEntry& b)
            {
                return a.priority > b.priority;
            });

            for (auto& m : m_modules)
                m.instance->shutdown();

            m_modules.clear();
        }

    private:
        struct ModuleEntry
        {
            std::type_index id;
            std::shared_ptr<IModule> instance;
            int priority;

            ModuleEntry(std::type_index i, std::shared_ptr<IModule> p, int pr)
                : id(i), instance(std::move(p)), priority(pr) {}
        };

        std::list<ModuleEntry> m_modules;
    };
}
