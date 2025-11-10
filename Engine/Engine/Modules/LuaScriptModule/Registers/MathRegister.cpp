#include "MathRegister.h"

#include "../LuaScriptModule.h"

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <format>

namespace ScriptModule
{
void MathRegister::registerTypes(LuaScriptModule&, sol::state& state)
{
    state.new_usertype<glm::vec2>(
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

    state.set_function("DotProduct2",
                       [](const glm::vec2& v1, const glm::vec2& v2) -> float { return glm::dot(v1, v2); });

    state.set_function("Distance2",
                       [](const glm::vec2& v1, const glm::vec2& v2) -> float { return glm::distance(v1, v2); });

    state.set_function("Lerp2", [](const glm::vec2& v1, const glm::vec2& v2, float t) -> glm::vec2
                       { return glm::mix(v1, v2, t); });

    state.set_function("Reflect2", [](const glm::vec2& v, const glm::vec2& normal) -> glm::vec2
                       { return glm::reflect(v, glm::normalize(normal)); });

    state.set_function("Rotate2",
                       [](const glm::vec2& v, float angle) -> glm::vec2
                       {
                           float cosA = glm::cos(angle);
                           float sinA = glm::sin(angle);
                           return {v.x * cosA - v.y * sinA, v.x * sinA + v.y * cosA};
                       });

    // Регистрируем userdata type для glm::vec3 с методами и операторами
    // Используем внутреннее имя типа, чтобы не конфликтовать с функцией-конструктором
    state.new_usertype<glm::vec3>(
        "Vec3Type",
        "x", &glm::vec3::x,
        "y", &glm::vec3::y,
        "z", &glm::vec3::z,
        "length", [](const glm::vec3& v) -> float { return glm::length(v); },
        "normalize", [](glm::vec3& v) { v = glm::normalize(v); },
        "getNormalized", [](const glm::vec3& v) -> glm::vec3 { return glm::normalize(v); },
        "__add", [](const glm::vec3& v1, const glm::vec3& v2) -> glm::vec3 { return v1 + v2; },
        "__sub", [](const glm::vec3& v1, const glm::vec3& v2) -> glm::vec3 { return v1 - v2; },
        "__mul", [](const glm::vec3& v, float scalar) -> glm::vec3 { return v * scalar; },
        "__div", [](const glm::vec3& v, float scalar) -> glm::vec3 { return v / scalar; },
        "__tostring", [](const glm::vec3& v) -> std::string { return std::format("Vec3({}, {}, {})", v.x, v.y, v.z); },
        "dot", [](const glm::vec3& v1, const glm::vec3& v2) -> float { return glm::dot(v1, v2); },
        "cross", [](const glm::vec3& v1, const glm::vec3& v2) -> glm::vec3 { return glm::cross(v1, v2); },
        "projectOnto", [](const glm::vec3& v1, const glm::vec3& v2) -> glm::vec3
        { return glm::dot(v1, glm::normalize(v2)) * glm::normalize(v2); },
        "reflect", [](const glm::vec3& v, const glm::vec3& normal) -> glm::vec3
        { return glm::reflect(v, glm::normalize(normal)); });
    
    // Регистрируем Vec3 как вызываемую функцию-конструктор
    // Это гарантирует, что Vec3() работает как функция в любом контексте (включая замыкания)
    state.set_function("Vec3", sol::overload(
        []() -> glm::vec3 { return glm::vec3(0.0f, 0.0f, 0.0f); },
        [](float x, float y, float z) -> glm::vec3 { return glm::vec3(x, y, z); },
        [](float val) -> glm::vec3 { return glm::vec3(val, val, val); }
    ));

    state.set_function("DotProduct3",
                       [](const glm::vec3& v1, const glm::vec3& v2) -> float { return glm::dot(v1, v2); });

    state.set_function("CrossProduct3",
                       [](const glm::vec3& v1, const glm::vec3& v2) -> glm::vec3 { return glm::cross(v1, v2); });

    state.set_function("Distance3",
                       [](const glm::vec3& v1, const glm::vec3& v2) -> float { return glm::distance(v1, v2); });

    state.set_function("Lerp3", [](const glm::vec3& v1, const glm::vec3& v2, float t) -> glm::vec3
                       { return glm::mix(v1, v2, t); });

    state.set_function("Reflect3", [](const glm::vec3& v, const glm::vec3& normal) -> glm::vec3
                       { return glm::reflect(v, glm::normalize(normal)); });
}
} // namespace ScriptModule

