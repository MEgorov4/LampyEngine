#include "DevScriptManager.h"

#include <Modules/ResourceModule/ResourceManager.h>
#include <Modules/ResourceModule/Script.h>

#include <Foundation/Log/LoggerMacro.h>

#include <format>

namespace ScriptModule
{
namespace
{
constexpr std::string_view kDevFolderPrefix = "Scripts/Dev";
constexpr std::string_view kDevManagerCategory = "DevScriptManager";
}

DevScriptManager::DevScriptManager(ResourceModule::ResourceManager* resourceManager,
                                   ResourceModule::AssetManager* assetManager) :
    m_resourceManager(resourceManager),
    m_assetManager(assetManager)
{
}

void DevScriptManager::loadAll(ScriptVM& devVM)
{
    if (!m_resourceManager)
    {
        LT_LOGW(kDevManagerCategory.data(), "ResourceManager not available, dev scripts disabled");
        return;
    }

    auto* database = m_resourceManager->getDatabase();
    if (!database)
    {
        LT_LOGW(kDevManagerCategory.data(), "AssetDatabase not available, dev scripts disabled");
        return;
    }

    database->forEach([this, &devVM](const ResourceModule::AssetID& guid, const ResourceModule::AssetInfo& info) {
        if (info.type != ResourceModule::AssetType::Script)
            return;

        if (info.sourcePath.empty())
            return;

        if (info.sourcePath.rfind(kDevFolderPrefix, 0) != 0)
            return;

        loadScript(devVM, guid, info.sourcePath);
    });
}

bool DevScriptManager::loadScript(ScriptVM& devVM, const ResourceModule::AssetID& id, std::string_view key)
{
    auto* script = loadResource(id);
    if (!script)
        return false;

    ScriptInstance instance{
        .id           = id,
        .key          = std::string(key),
        .environment  = sol::environment(devVM.state(), sol::create, devVM.environment()),
        .exports      = devVM.state().create_table()};

    if (!executeChunk(devVM, instance, script))
        return false;

    m_keyLookup[instance.key] = id;
    m_scripts[id]             = instance;

    if (m_autoExecEnabled)
    {
        autoInvoke(m_scripts[id]);
    }

    LT_LOGI(kDevManagerCategory.data(), std::format("Loaded dev script {}", instance.key));
    return true;
}

bool DevScriptManager::reloadScript(ScriptVM& devVM, const ResourceModule::AssetID& id)
{
    auto it = m_scripts.find(id);
    if (it == m_scripts.end())
        return false;

    const std::string key = it->second.key;
    m_scripts.erase(it);
    return loadScript(devVM, id, key);
}

bool DevScriptManager::reloadScript(ScriptVM& devVM, std::string_view key)
{
    std::string keyStr{key};
    auto it = m_keyLookup.find(keyStr);
    if (it == m_keyLookup.end())
        return false;

    return reloadScript(devVM, it->second);
}

void DevScriptManager::clear()
{
    m_scripts.clear();
    m_keyLookup.clear();
}

bool DevScriptManager::hasScript(std::string_view key) const noexcept
{
    std::string keyStr{key};
    return m_keyLookup.find(keyStr) != m_keyLookup.end();
}

const sol::table* DevScriptManager::getExports(std::string_view key) const
{
    std::string keyStr{key};
    auto keyIt = m_keyLookup.find(keyStr);
    if (keyIt == m_keyLookup.end())
        return nullptr;

    auto instanceIt = m_scripts.find(keyIt->second);
    if (instanceIt == m_scripts.end())
        return nullptr;

    return &instanceIt->second.exports;
}

std::vector<std::string> DevScriptManager::listScripts() const
{
    std::vector<std::string> names;
    names.reserve(m_keyLookup.size());
    for (const auto& [key, _] : m_keyLookup)
    {
        names.push_back(key);
    }
    return names;
}

ResourceModule::RScript* DevScriptManager::loadResource(const ResourceModule::AssetID& id)
{
    if (!m_resourceManager)
        return nullptr;

    auto resource = m_resourceManager->load<ResourceModule::RScript>(id);
    if (!resource)
    {
        LT_LOGE(kDevManagerCategory.data(), std::format("Failed to load dev script [{}]", id.str()));
        return nullptr;
    }

    return resource.get();
}

bool DevScriptManager::executeChunk(ScriptVM& devVM, ScriptInstance& instance, ResourceModule::RScript* script)
{
    sol::load_result chunk = devVM.state().load(script->getSource());
    if (!chunk.valid())
    {
        sol::error err = chunk;
        LT_LOGE(kDevManagerCategory.data(), std::format("Lua load error in dev script {}: {}", instance.key, err.what()));
        return false;
    }

    sol::function func = chunk;
    sol::set_environment(devVM.environment(), func);
    sol::protected_function protectedFunc = func;
    sol::protected_function_result exec = protectedFunc();
    if (!exec.valid())
    {
        sol::error err = exec;
        LT_LOGE(
            kDevManagerCategory.data(),
            std::format("Lua execution error in dev script {}: {}", instance.key, err.what()));
        return false;
    }

    sol::table exports;
    if (exec.return_count() > 0)
    {
        sol::object ret = exec.get<sol::object>();
        if (ret.valid() && ret.get_type() == sol::type::table)
        {
            exports = ret.as<sol::table>();
        }
    }

    if (!exports.valid())
    {
        exports = devVM.state().create_table();
    }

    instance.exports = exports;
    return true;
}

void DevScriptManager::autoInvoke(ScriptInstance& instance)
{
    auto invokeIfValid = [&instance](std::string_view fnName) {
        sol::object candidate = instance.exports[std::string(fnName)];
        if (!candidate.valid() || candidate.get_type() != sol::type::function)
            return;

        sol::protected_function func = candidate;
        sol::protected_function_result result = func();
        if (!result.valid())
        {
            sol::error err = result;
            LT_LOGW(kDevManagerCategory.data(),
                    std::format("Auto invoke error in {}::{} - {}", instance.key, fnName, err.what()));
        }
    };

    invokeIfValid("onLoad");
    invokeIfValid("start");
}
} // namespace ScriptModule

