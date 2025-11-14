#pragma once

#include "ScriptVM.h"

#include <Modules/ResourceModule/Asset/AssetID.h>
#include <Modules/ResourceModule/Asset/AssetInfo.h>

#include <sol/table.hpp>

#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

namespace ResourceModule
{
class ResourceManager;
class AssetManager;
class RScript;
}

namespace ScriptModule
{
class DevScriptManager
{
  public:
    DevScriptManager(ResourceModule::ResourceManager* resourceManager, ResourceModule::AssetManager* assetManager);

    void setAutoExecEnabled(bool enabled) noexcept { m_autoExecEnabled = enabled; }
    bool isAutoExecEnabled() const noexcept { return m_autoExecEnabled; }

    void loadAll(ScriptVM& devVM);
    bool loadScript(ScriptVM& devVM, const ResourceModule::AssetID& id, std::string_view key);
    bool reloadScript(ScriptVM& devVM, const ResourceModule::AssetID& id);
    bool reloadScript(ScriptVM& devVM, std::string_view key);
    void clear();

    bool hasScript(std::string_view key) const noexcept;

    const sol::table* getExports(std::string_view key) const;
    std::vector<std::string> listScripts() const;

  private:
    struct ScriptInstance
    {
        ResourceModule::AssetID id;
        std::string key;
        sol::environment environment;
        sol::table exports;
    };

    ResourceModule::RScript* loadResource(const ResourceModule::AssetID& id);
    bool executeChunk(ScriptVM& devVM, ScriptInstance& instance, ResourceModule::RScript* script);
    void autoInvoke(ScriptInstance& instance);

  private:
    ResourceModule::ResourceManager* m_resourceManager{nullptr};
    ResourceModule::AssetManager* m_assetManager{nullptr};
    std::unordered_map<ResourceModule::AssetID, ScriptInstance, ResourceModule::AssetID::Hasher> m_scripts;
    std::unordered_map<std::string, ResourceModule::AssetID> m_keyLookup;
    bool m_autoExecEnabled{true};
};
} // namespace ScriptModule

