#include "RenderComponentsRegister.h"

#include "ECSBindingUtils.h"

namespace ScriptModule
{
void RenderComponentsRegister::registerTypes(sol::state& state, sol::environment& env)
{
    state.new_usertype<CameraComponent>(
        "CameraComponent",
        sol::constructors<CameraComponent()>(),
        "fov", &CameraComponent::fov,
        "aspect", &CameraComponent::aspect,
        "near_clip", &CameraComponent::nearClip,
        "far_clip", &CameraComponent::farClip,
        "is_viewport_camera", &CameraComponent::isViewportCamera);

    state.new_usertype<MeshComponent>(
        "MeshComponentType",
        "mesh_id", &MeshComponent::meshID,
        "texture_id", &MeshComponent::textureID,
        "vert_shader_id", &MeshComponent::vertShaderID,
        "frag_shader_id", &MeshComponent::fragShaderID);

    sol::table envTable  = env;
    sol::table meshCtor  = state.create_table();
    meshCtor["new"]      = []() -> MeshComponent { return MeshComponent(); };
    envTable["MeshComponent"] = meshCtor;

    state.new_usertype<MaterialComponent>(
        "MaterialComponentType",
        "material_id", &MaterialComponent::materialID);

    sol::table materialComponentTable = state.create_table();
    materialComponentTable["new"]     = []() -> MaterialComponent { return MaterialComponent(); };
    envTable["MaterialComponent"]     = materialComponentTable;
}
} // namespace ScriptModule

