#pragma once

#include <functional>
#include <memory>
#include <typeindex>
#include <utility>
#include <unordered_map>

class ModuleConfigRegistry
{
    struct ConfigEntry
    {
        std::shared_ptr<void> configData;
        std::function<void(void*, void*)> applyFunc;
    };

    std::unordered_map<std::type_index, ConfigEntry> m_entries;

  public:
    ModuleConfigRegistry() = default;

    template <typename ModuleT, typename ConfigT>
    void setConfig(ConfigT config)
    {
        ConfigEntry entry;
        entry.configData = std::make_shared<ConfigT>(std::move(config));
        entry.applyFunc = [](void *modulePtr, void *configPtr) {
            if (!modulePtr || !configPtr)
                return;
            auto *module = static_cast<ModuleT *>(modulePtr);
            auto *cfg = static_cast<ConfigT *>(configPtr);
            module->applyConfig(*cfg);
        };

        m_entries[std::type_index(typeid(ModuleT))] = std::move(entry);
    }

    template <typename ModuleT>
    void applyConfig(ModuleT &module) const
    {
        auto it = m_entries.find(std::type_index(typeid(ModuleT)));
        if (it == m_entries.end())
            return;

        const ConfigEntry &entry = it->second;
        entry.applyFunc(static_cast<void *>(&module), entry.configData.get());
    }

    template <typename ModuleT>
    bool hasConfig() const
    {
        return m_entries.find(std::type_index(typeid(ModuleT))) != m_entries.end();
    }
};

