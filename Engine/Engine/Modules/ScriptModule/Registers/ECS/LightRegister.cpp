#include "LightRegister.h"

#include "ECSBindingUtils.h"

namespace ScriptModule
{
void LightRegister::registerTypes(sol::state& state, sol::environment& env)
{
    state.new_usertype<PointLightComponent>(
        "PointLightComponentType",
        "inner_radius", &PointLightComponent::innerRadius,
        "outer_radius", &PointLightComponent::outerRadius,
        "intensity", &PointLightComponent::intencity,
        "color", &PointLightComponent::color);

    sol::table pointCtor = state.create_table();
    pointCtor["new"]      = []() -> PointLightComponent { return PointLightComponent(); };
    sol::table envTable   = env;
    envTable["PointLightComponent"] = pointCtor;

    state.new_usertype<DirectionalLightComponent>(
        "DirectionalLightComponent",
        sol::constructors<DirectionalLightComponent()>(),
        "intensity", &DirectionalLightComponent::intencity);
}
} // namespace ScriptModule

