#pragma once
#include "Core.h"
#include "IModule.h"
#include <algorithm>
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <unordered_map>
#include <vector>

namespace EngineCore::Base
{
    class ContextLocator final {
    public:
        explicit ContextLocator(CoreLocator& core) : m_core(core) {}

        template <typename T>
        void registerMinor(std::shared_ptr<T> module, int priority = 0) {
            static_assert(std::is_base_of_v<IModule, T>, "T must inherit from IModule");
            m_minor.emplace_back(typeid(T), std::move(module), priority);
        }

        template <typename T>
        void registerMajor(std::shared_ptr<T> module, int priority = 0) {
            static_assert(std::is_base_of_v<IModule, T>, "T must inherit from IModule");
            m_major.emplace_back(typeid(T), std::move(module), priority);
        }

        template <typename T> T& get() {
            for (auto& m : m_minor)
                if (m.id == typeid(T))
                    return *std::static_pointer_cast<T>(m.instance);
            for (auto& m : m_major)
                if (m.id == typeid(T))
                    return *std::static_pointer_cast<T>(m.instance);
            return m_core.get<T>();
        }

        void startupMinor() { startList(m_minor); }
        void startupMajor() { startList(m_major); }

        void shutdownAll() {
            shutdownList(m_major);
            shutdownList(m_minor);
            m_major.clear();
            m_minor.clear();
        }

    private:
        struct Entry {
            std::type_index id;
            std::shared_ptr<IModule> instance;
            int priority;
            Entry(std::type_index i, std::shared_ptr<IModule> p, int pr)
                : id(i), instance(std::move(p)), priority(pr) {
            }
        };

        CoreLocator& m_core;
        std::vector<Entry> m_minor;
        std::vector<Entry> m_major;

        static void startList(std::vector<Entry>& list) {
            std::sort(list.begin(), list.end(),
                [](auto& a, auto& b) { return a.priority < b.priority; });
            for (auto& m : list)
                m.instance->startup();
        }

        static void shutdownList(std::vector<Entry>& list) {
            std::sort(list.begin(), list.end(),
                [](auto& a, auto& b) { return a.priority > b.priority; });
            for (auto& m : list)
                m.instance->shutdown();
        }
    };
}
