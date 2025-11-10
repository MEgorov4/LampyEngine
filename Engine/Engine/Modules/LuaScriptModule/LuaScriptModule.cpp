#include "LuaScriptModule.h"

#include "Registers/FoundationRegister.h"
#include "Registers/MathRegister.h"
#include "Registers/ECSRegister.h"
#include "Registers/ImGUIRegister.h"

#include <Modules/AudioModule/AudioModule.h>
#include <Modules/InputModule/InputModule.h>
#include <Modules/ObjectCoreModule/ECS/Components/ECSComponents.h>
#include <Modules/ObjectCoreModule/ECS/ECSModule.h>
#include <Modules/ResourceModule/Asset/AssetDatabase.h>
#include <Modules/ResourceModule/ResourceManager.h>
#include <Modules/ResourceModule/Script.h>
#include <Modules/ResourceModule/Asset/AssetManager.h>

#include <format>
#include <algorithm>
#include <cctype>

namespace ScriptModule
{
namespace
{
constexpr std::string_view kDevScriptPrefix = "Scripts/Dev";
}

void LuaScriptModule::startup()
{
    m_inputModule = GCM(InputModule::InputModule);
    m_audioModule = GCM(AudioModule::AudioModule);
    m_ecsModule   = GCM(ECSModule::ECSModule);

    m_luaState.open_libraries(sol::lib::base, sol::lib::package, sol::lib::os, sol::lib::string);

    initializeRegisters();
    for (auto& reg : m_registers)
    {
        reg->registerTypes(*this, m_luaState);
    }
    
    // Проверяем, что Vec3 зарегистрирован как вызываемый тип
    // Это гарантирует, что скрипты смогут использовать Vec3() как функцию
    try
    {
        sol::object vec3Obj = m_luaState["Vec3"];
        if (vec3Obj.valid())
        {
            LT_LOGI("LuaScriptModule", "Vec3 is registered in global scope");
        }
        else
        {
            LT_LOGW("LuaScriptModule", "Vec3 is NOT registered in global scope!");
        }
    }
    catch (...)
    {
        LT_LOGW("LuaScriptModule", "Failed to check Vec3 registration");
    }

    loadDevScriptsFromDatabase();

    if (auto* assetManager = GCM(ResourceModule::AssetManager))
    {
        m_assetImportedSubscription = assetManager->OnAssetImported.subscribe(
            [this](const ResourceModule::AssetInfo& info)
            {
                if (info.type != ResourceModule::AssetType::Script)
                    return;

                std::string_view source{info.sourcePath};
                if (source.rfind(kDevScriptPrefix, 0) != 0)
                    return;

                if (!reloadDevScript(source))
                {
                    loadDevScript(info.guid, source);
                }
            });
    }
}

void LuaScriptModule::shutdown()
{
    LT_LOGI("LuaScriptModule", "Shutdown");
    clearDevScripts();
    m_assetImportedSubscription.unsubscribe();
    m_registers.clear();
    // Не вызываем lua_close() вручную - sol::state сам управляет Lua state
    // и корректно закроет его в деструкторе, освободив все ссылки перед закрытием
    // Вызов lua_close() здесь приводит к тому, что деструктор sol::state пытается
    // освободить уже закрытый Lua state, что вызывает краш
}

void LuaScriptModule::processCommand(const std::string& command)
{
    try
    {
        sol::load_result loadedScript = m_luaState.load(command);
        if (!loadedScript.valid())
        {
            sol::error loadError = loadedScript;
            return;
        }

        sol::protected_function script        = loadedScript;
        sol::protected_function_result result = script();

        if (!result.valid())
        {
            sol::error execError = result;
        }
    }
    catch (const sol::error& e)
    {
        LT_LOGE("LuaScriptModule", "Lua exception caught: " + std::string(e.what()));
    }
}

bool LuaScriptModule::hasDevScript(std::string_view name) const noexcept
{
    std::string key{name};
    return m_devScripts.find(key) != m_devScripts.end();
}

std::vector<std::string> LuaScriptModule::listDevScripts() const
{
    std::vector<std::string> result;
    result.reserve(m_devScripts.size());
    for (const auto& [key, _] : m_devScripts)
        result.push_back(key);
    return result;
}

bool LuaScriptModule::reloadDevScript(std::string_view name)
{
    std::string key{name};
    auto it = m_devScripts.find(key);
    if (it == m_devScripts.end())
        return false;

    ResourceModule::AssetID id = it->second.id;
    m_devScripts.erase(it);
    return loadDevScript(id, key);
}

void LuaScriptModule::clearDevScripts()
{
    m_devScripts.clear();
}

void LuaScriptModule::loadDevScriptsFromDatabase()
{
    auto* resourceManager = GCM(ResourceModule::ResourceManager);
    if (!resourceManager)
    {
        LT_LOGW("LuaScriptModule", "ResourceManager not available, dev scripts disabled");
        return;
    }

    auto* database = resourceManager->getDatabase();
    if (!database)
    {
        LT_LOGW("LuaScriptModule", "AssetDatabase not set, dev scripts disabled");
        return;
    }

    database->forEach([this](const ResourceModule::AssetID& guid, const ResourceModule::AssetInfo& info) {
        if (info.type != ResourceModule::AssetType::Script)
            return;

        if (info.sourcePath.empty())
            return;

        std::string_view source{info.sourcePath};
        if (source.rfind(kDevScriptPrefix, 0) != 0)
            return;

        loadDevScript(guid, source);
    });
}

bool LuaScriptModule::loadDevScript(const ResourceModule::AssetID& id, std::string_view key)
{
    auto* resourceManager = GCM(ResourceModule::ResourceManager);
    if (!resourceManager)
    {
        LT_LOGE("LuaScriptModule", "Cannot load dev script without ResourceManager");
        return false;
    }

    auto script = resourceManager->load<ResourceModule::RScript>(id);
    if (!script)
    {
        LT_LOGE("LuaScriptModule", std::format("Failed to load dev script resource [{}]", id.str()));
        return false;
    }

    // РЕШЕНИЕ: Выполняем скрипт через load + call в глобальном scope
    // Это гарантирует, что скрипт выполняется в правильном контексте
    sol::load_result loaded = m_luaState.load(script->getSource());
    if (!loaded.valid())
    {
        sol::error err = loaded;
        LT_LOGE("LuaScriptModule", std::format("Lua load error in dev script {}: {}", key, err.what()));
        return false;
    }
    
    sol::protected_function chunk = loaded;
    sol::protected_function_result exec = chunk();
    if (!exec.valid())
    {
        sol::error err = exec;
        LT_LOGE("LuaScriptModule", std::format("Lua execution error in dev script {}: {}", key, err.what()));
        return false;
    }

    // Получаем возвращаемое значение (таблица exports или nil)
    sol::table exports;
    if (exec.return_count() > 0)
    {
        sol::object ret = exec.get<sol::object>();
        if (ret.valid() && ret.get_type() == sol::type::table)
        {
            exports = ret.as<sol::table>();
        }
    }

    // Если скрипт не вернул таблицу, создаем пустую таблицу для exports
    if (!exports.valid())
    {
        exports = m_luaState.create_table();
    }

    // Создаем пустой environment для совместимости (не используется, но нужен для структуры)
    sol::environment env(m_luaState, sol::create);

    std::string keyStr{key};
    DevScriptInstance instance{
        id,
        keyStr,
        env,
        exports};

    m_devScripts[keyStr] = std::move(instance);
    LT_LOGI("LuaScriptModule", std::format("Loaded dev script: {}", key));
    
    // Автоматически вызываем функцию onLoad() или start() если она есть
    // Скрипт выполнен в глобальном scope, поэтому функции имеют прямой доступ ко всем глобальным переменным
    // Вызываем функцию напрямую через sol2 - она уже создана в правильном контексте
    try
    {
        sol::object onLoadObj = exports.get<sol::object>("onLoad");
        if (!onLoadObj.valid() || onLoadObj.get_type() != sol::type::function)
        {
            onLoadObj = exports.get<sol::object>("start");
        }
        
        if (onLoadObj.valid() && onLoadObj.get_type() == sol::type::function)
        {
            sol::protected_function func = onLoadObj.as<sol::protected_function>();
            // Функция создана в глобальном scope, поэтому имеет прямой доступ к Vec3, GetCurrentWorld, etc.
            sol::protected_function_result result = func();
            if (!result.valid())
            {
                sol::error err = result;
                LT_LOGW("LuaScriptModule", std::format("Error calling onLoad/start in dev script {}: {}", key, err.what()));
            }
            else
            {
                LT_LOGI("LuaScriptModule", std::format("Auto-executed onLoad/start in dev script: {}", key));
            }
        }
    }
    catch (const std::exception& e)
    {
        LT_LOGW("LuaScriptModule", std::format("Exception while auto-executing onLoad/start in dev script {}: {}", key, e.what()));
    }
    catch (...)
    {
        LT_LOGW("LuaScriptModule", std::format("Unknown exception while auto-executing onLoad/start in dev script: {}", key));
    }
    
    return true;
}

void LuaScriptModule::initializeRegisters()
{
    m_registers.clear();
    m_registers.emplace_back(std::make_unique<FoundationRegister>());
    m_registers.emplace_back(std::make_unique<MathRegister>());
    m_registers.emplace_back(std::make_unique<ECSRegister>());
    m_registers.emplace_back(std::make_unique<ImGUIRegister>());
}
} // namespace ScriptModule
