#include "LuaScriptModule.h"

#include <Modules/AudioModule/AudioModule.h>
#include <Modules/InputModule/InputModule.h>
#include <Modules/ObjectCoreModule/ECS/Components/ECSComponents.h>
#include <Modules/ObjectCoreModule/ECS/ECSModule.h>
#include <sol/sol.hpp>

namespace ScriptModule
{
void LuaScriptModule::startup()
{
    m_inputModule = GCM(InputModule::InputModule);
    m_audioModule = GCM(AudioModule::AudioModule);
    m_ecsModule   = GCM(ECSModule::ECSModule);

    m_luaState.open_libraries(sol::lib::base, sol::lib::package, sol::lib::os, sol::lib::string);

    registerLogger();

    registerEvent();
    registerInputModuleEvents();

    registerAudioModule();

    registerMathTypes();

    registerECSModule();
}

void LuaScriptModule::shutdown()
{
    LT_LOGI("LuaScriptModule", "Shutdown");
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

void LuaScriptModule::registerLogger()
{
    m_luaState.set_function("LogInfo", [](const std::string& msg) { LT_LOG(LogVerbosity::Info, "Script", msg); });
    m_luaState.set_function("LogDebug", [](const std::string& msg) { LT_LOG(LogVerbosity::Debug, "Script", msg); });
    m_luaState.set_function("LogVerbose", [](const std::string& msg) { LT_LOG(LogVerbosity::Verbose, "Script", msg); });
    m_luaState.set_function("LogWarning", [](const std::string& msg) { LT_LOG(LogVerbosity::Warning, "Script", msg); });
    m_luaState.set_function("LogError", [](const std::string& msg) { LT_LOG(LogVerbosity::Error, "Script", msg); });
    m_luaState.set_function("LogFatal", [](const std::string& msg) { LT_LOG(LogVerbosity::Fatal, "Script", msg); });
}

void LuaScriptModule::registerEvent()
{
    m_luaState.new_usertype<Event<SDL_KeyboardEvent>>(
        "EventKeyboard", sol::constructors<Event<SDL_KeyboardEvent>()>(),

        "subscribe", [](Event<SDL_KeyboardEvent>& self, sol::function luaHandler)
        { return self.subscribe([luaHandler](SDL_KeyboardEvent keyboardEvent) { luaHandler(keyboardEvent); }); },

        "unsubscribe", &Event<SDL_KeyboardEvent>::unsubscribe,

        "invoke", &Event<SDL_KeyboardEvent>::operator());
}

void LuaScriptModule::registerInputModuleEvents()
{
    m_luaState["OnKeyboardEvent"] = &m_inputModule->OnKeyboardEvent;
    /*
    m_luaState["OnMouseButtonEvent"] = &m_inputModule->OnMouseButtonEvent;
    m_luaState["OnMouseMotionEvent"] = &m_inputModule->OnMouseMotionEvent;
    m_luaState["OnMouseWheelEvent"] = &m_inputModule->OnMouseWheelEvent;
*/
}

void LuaScriptModule::registerAudioModule()
{
    // m_luaState.set_function("PlaySoundAsync", []() {
    //	AudioModule::getInstance().playSoundAsync();
    //	});
}

void LuaScriptModule::registerMathTypes()
{
    m_luaState.new_usertype<glm::vec2>(
        "Vec2", sol::constructors<glm::vec2(), glm::vec2(float, float)>(), "x", &glm::vec2::x, "y", &glm::vec2::y,

        "length", [](const glm::vec2& v) -> float { return glm::length(v); },

        "normalize", [](glm::vec2& v) { v = glm::normalize(v); },

        "__add", [](const glm::vec2& v1, const glm::vec2& v2) -> glm::vec2 { return v1 + v2; },

        "__sub", [](const glm::vec2& v1, const glm::vec2& v2) -> glm::vec2 { return v1 - v2; },

        "__mul", [](const glm::vec2& v, float scalar) -> glm::vec2 { return v * scalar; },

        "__div", [](const glm::vec2& v, float scalar) -> glm::vec2 { return v / scalar; },

        "__tostring", [](const glm::vec2& v) -> std::string { return std::format("Vec2({}, {})", v.x, v.y); },

        "projectOnto", [](const glm::vec2& v1, const glm::vec2& v2) -> glm::vec2
        { return glm::dot(v1, glm::normalize(v2)) * glm::normalize(v2); },

        "perpendicular", [](const glm::vec2& v) -> glm::vec2 { return glm::vec2(-v.y, v.x); },

        "getNormalized", [](const glm::vec2& v) -> glm::vec2 { return glm::normalize(v); });

    m_luaState.set_function("DotProduct2",
                            [](const glm::vec2& v1, const glm::vec2& v2) -> float { return glm::dot(v1, v2); });

    m_luaState.set_function("Distance2",
                            [](const glm::vec2& v1, const glm::vec2& v2) -> float { return glm::distance(v1, v2); });

    m_luaState.set_function("Lerp2", [](const glm::vec2& v1, const glm::vec2& v2, float t) -> glm::vec2
                            { return glm::mix(v1, v2, t); });

    m_luaState.set_function("Reflect2", [](const glm::vec2& v, const glm::vec2& normal) -> glm::vec2
                            { return glm::reflect(v, glm::normalize(normal)); });

    m_luaState.set_function("Rotate2",
                            [](const glm::vec2& v, float angle) -> glm::vec2
                            {
                                float cosA = glm::cos(angle);
                                float sinA = glm::sin(angle);
                                return {v.x * cosA - v.y * sinA, v.x * sinA + v.y * cosA};
                            });

    m_luaState.new_usertype<glm::vec3>(
        "Vec3", sol::constructors<glm::vec3(), glm::vec3(float, float, float)>(), "x", &glm::vec3::x, "y",
        &glm::vec3::y, "z", &glm::vec3::z,

        "length", [](const glm::vec3& v) -> float { return glm::length(v); },

        "normalize", [](glm::vec3& v) { v = glm::normalize(v); },

        "getNormalized", [](const glm::vec3& v) -> glm::vec3 { return glm::normalize(v); },

        "__add", [](const glm::vec3& v1, const glm::vec3& v2) -> glm::vec3 { return v1 + v2; },

        "__sub", [](const glm::vec3& v1, const glm::vec3& v2) -> glm::vec3 { return v1 - v2; },

        "__mul", [](const glm::vec3& v, float scalar) -> glm::vec3 { return v * scalar; },

        "__div", [](const glm::vec3& v, float scalar) -> glm::vec3 { return v / scalar; },

        "__tostring", [](const glm::vec3& v) -> std::string { return std::format("Vec2({}, {}, {})", v.x, v.y, v.z); },

        "dot", [](const glm::vec3& v1, const glm::vec3& v2) -> float { return glm::dot(v1, v2); },

        "cross", [](const glm::vec3& v1, const glm::vec3& v2) -> glm::vec3 { return glm::cross(v1, v2); },

        "projectOnto", [](const glm::vec3& v1, const glm::vec3& v2) -> glm::vec3
        { return glm::dot(v1, glm::normalize(v2)) * glm::normalize(v2); },

        "reflect", [](const glm::vec3& v, const glm::vec3& normal) -> glm::vec3
        { return glm::reflect(v, glm::normalize(normal)); });

    m_luaState.set_function("DotProduct3",
                            [](const glm::vec3& v1, const glm::vec3& v2) -> float { return glm::dot(v1, v2); });

    m_luaState.set_function("CrossProduct3",
                            [](const glm::vec3& v1, const glm::vec3& v2) -> glm::vec3 { return glm::cross(v1, v2); });

    m_luaState.set_function("Distance3",
                            [](const glm::vec3& v1, const glm::vec3& v2) -> float { return glm::distance(v1, v2); });

    m_luaState.set_function("Lerp3", [](const glm::vec3& v1, const glm::vec3& v2, float t) -> glm::vec3
                            { return glm::mix(v1, v2, t); });

    m_luaState.set_function("Reflect3", [](const glm::vec3& v, const glm::vec3& normal) -> glm::vec3
                            { return glm::reflect(v, glm::normalize(normal)); });
}

void LuaScriptModule::registerECSModule()
{
    using namespace ECSModule;
    m_luaState.new_usertype<flecs::world>(
        "World", "entity",
        [](flecs::world& w, const std::string& name) -> flecs::entity
        {
            auto entity = w.entity(name.c_str());
            entity.set<PositionComponent>({});
            return entity;
        },
        "find", [](flecs::world& w, const std::string& name) -> flecs::entity { return w.lookup(name.c_str()); });

    m_luaState.new_usertype<PositionComponent>("PositionComponent", "x", &PositionComponent::x, "y",
                                               &PositionComponent::y, "z", &PositionComponent::z);

    //m_luaState.new_usertype<flecs::entity>(
    //    "Entity", "id", &flecs::entity::id, "destruct", &flecs::entity::destruct, "is_valid", &flecs::entity::is_valid,

    //    "get_name", [](flecs::entity& e) -> const char* { return e.name().c_str(); },

    //    "get_position",
    //    [this](flecs::entity& e) -> glm::vec3
    //    {
    //        if (auto* position = e.get<PositionComponent>())
    //        {
    //            return position->toGLMVec();
    //        }
    //        return glm::vec3();
    //    },

    //    "set_position",
    //    [](flecs::entity& e, glm::vec3 pos)
    //    {
    //        if (e.has<PositionComponent>())
    //        {
    //            e.set<PositionComponent>({pos.x, pos.y, pos.z});
    //        }
    //    });

    //m_luaState.set_function("GetWorld", [this]() -> flecs::world& { return m_ecsModule->getCurrentWorld(); });
}
} // namespace ScriptModule
