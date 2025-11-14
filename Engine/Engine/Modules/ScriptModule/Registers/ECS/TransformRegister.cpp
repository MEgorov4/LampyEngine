#include "TransformRegister.h"

#include "ECSBindingUtils.h"

namespace ScriptModule
{
void TransformRegister::registerTypes(sol::state& state, sol::environment& env)
{
    state.new_usertype<PositionComponent>(
        "PositionComponent",
        sol::constructors<PositionComponent()>(),
        "x", &PositionComponent::x,
        "y", &PositionComponent::y,
        "z", &PositionComponent::z,
        "to_vec3", [](const PositionComponent& self) { return self.toGLMVec(); },
        "from_vec3", [](PositionComponent& self, const glm::vec3& v) { self.fromGLMVec(v); });

    state.new_usertype<RotationComponent>(
        "RotationComponentType",
        "x", &RotationComponent::x,
        "y", &RotationComponent::y,
        "z", &RotationComponent::z,
        "qx", &RotationComponent::qx,
        "qy", &RotationComponent::qy,
        "qz", &RotationComponent::qz,
        "qw", &RotationComponent::qw,
        "to_euler", [](const RotationComponent& self) { return self.toEulerDegrees(); },
        "from_euler", [](RotationComponent& self, const glm::vec3& eulerDeg) { self.fromEulerDegrees(eulerDeg); });

    env.set_function("RotationComponent", []() -> RotationComponent { return RotationComponent(); });

    state.new_usertype<ScaleComponent>(
        "ScaleComponent",
        sol::constructors<ScaleComponent()>(),
        "x", &ScaleComponent::x,
        "y", &ScaleComponent::y,
        "z", &ScaleComponent::z,
        "to_vec3", [](const ScaleComponent& self) { return self.toGLMVec(); },
        "from_vec3", [](ScaleComponent& self, const glm::vec3& v) { self.fromGMLVec(v); });
}
} // namespace ScriptModule

