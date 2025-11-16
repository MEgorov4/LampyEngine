#include "EntityRegister.h"

#include "ECSBindingUtils.h"

namespace ScriptModule
{
namespace
{
glm::vec3 ToVec3(const ScaleComponent& component)
{
    return component.toGLMVec();
}
} // namespace

void EntityRegister::registerTypes(sol::state& state, sol::environment& env)
{
    state.new_usertype<flecs::entity>(
        "Entity",
        "is_valid", [](const flecs::entity& e) { return e.is_alive(); },
        "id", [](const flecs::entity& e) { return static_cast<uint64_t>(e.id()); },
        "name",
        [](const flecs::entity& e) -> std::string
        {
            const char* n = e.name();
            return n ? std::string(n) : std::string();
        },
        "set_name", [](flecs::entity& e, const std::string& name) { e.set_name(name.c_str()); },
        "destruct", [](flecs::entity& e) { e.destruct(); },
        "has_transform", [](const flecs::entity& e) { return e.has<TransformComponent>(); },
        "get_transform",
        [](flecs::entity& e) -> sol::optional<TransformComponent>
        {
            if (const TransformComponent* transform = e.get<TransformComponent>())
                return *transform;
            return sol::nullopt;
        },
        "set_transform",
        [](flecs::entity& e, const TransformComponent& transform)
        {
            e.set<TransformComponent>(transform);
        },
        "remove_transform", [](flecs::entity& e) { RemoveComponentIfPresent<TransformComponent>(e); },
        "add_position",
        [](flecs::entity& e, const glm::vec3& value)
        {
            auto& transform = EnsureTransformComponent(e);
            transform.position.fromGLMVec(value);
            e.modified<TransformComponent>();
        },
        "remove_position", [](flecs::entity& e) { RemoveComponentIfPresent<TransformComponent>(e); },
        "has_position", [](const flecs::entity& e) { return e.has<TransformComponent>(); },
        "get_position",
        [](flecs::entity& e) -> sol::optional<glm::vec3>
        {
            if (const TransformComponent* transform = e.get<TransformComponent>())
                return transform->position.toGLMVec();
            return sol::nullopt;
        },
        "set_position",
        [](flecs::entity& e, const glm::vec3& value)
        {
            auto& transform = EnsureTransformComponent(e);
            transform.position.fromGLMVec(value);
            e.modified<TransformComponent>();
        },
        "has_rotation", [](const flecs::entity& e) { return e.has<TransformComponent>(); },
        "get_rotation",
        [](flecs::entity& e) -> sol::optional<RotationComponent>
        {
            if (const TransformComponent* transform = e.get<TransformComponent>())
                return transform->rotation;
            return sol::nullopt;
        },
        "set_rotation",
        [](flecs::entity& e, const RotationComponent& value)
        {
            auto& transform = EnsureTransformComponent(e);
            transform.rotation = value;
            e.modified<TransformComponent>();
        },
        "remove_rotation", [](flecs::entity& e) { RemoveComponentIfPresent<TransformComponent>(e); },
        "has_scale", [](const flecs::entity& e) { return e.has<TransformComponent>(); },
        "get_scale",
        [](flecs::entity& e) -> sol::optional<glm::vec3>
        {
            if (const TransformComponent* transform = e.get<TransformComponent>())
                return transform->scale.toGLMVec();
            return sol::nullopt;
        },
        "set_scale",
        [](flecs::entity& e, const glm::vec3& value)
        {
            auto& transform = EnsureTransformComponent(e);
            transform.scale.fromGMLVec(value);
            e.modified<TransformComponent>();
        },
        "remove_scale", [](flecs::entity& e) { RemoveComponentIfPresent<TransformComponent>(e); },
        "has_camera", [](const flecs::entity& e) { return e.has<CameraComponent>(); },
        "get_camera",
        [](flecs::entity& e) -> sol::optional<CameraComponent>
        {
            if (const CameraComponent* cam = e.get<CameraComponent>())
                return *cam;
            return sol::nullopt;
        },
        "set_camera",
        [](flecs::entity& e, const CameraComponent& cam)
        {
            e.set<CameraComponent>(cam);
        },
        "remove_camera", [](flecs::entity& e) { RemoveComponentIfPresent<CameraComponent>(e); },
        "has_mesh", [](const flecs::entity& e) { return e.has<MeshComponent>(); },
        "get_mesh",
        [](flecs::entity& e) -> sol::optional<MeshComponent>
        {
            if (const MeshComponent* mesh = e.get<MeshComponent>())
                return *mesh;
            return sol::nullopt;
        },
        "set_mesh",
        sol::overload(
            [](flecs::entity& e, const MeshComponent& mesh)
            {
                e.set<MeshComponent>(mesh);
            },
            [](flecs::entity& e, sol::table meshTable)
            {
                MeshComponent mesh{};
                if (meshTable["mesh_id"].valid())
                {
                    sol::object meshIdObj = meshTable["mesh_id"];
                    if (meshIdObj.is<std::string>())
                    {
                        mesh.meshID = ResourceModule::AssetID(meshIdObj.as<std::string>());
                    }
                    else if (meshIdObj.is<ResourceModule::AssetID>())
                    {
                        mesh.meshID = meshIdObj.as<ResourceModule::AssetID>();
                    }
                }
                if (meshTable["texture_id"].valid())
                {
                    sol::object textureIdObj = meshTable["texture_id"];
                    if (textureIdObj.is<std::string>())
                    {
                        mesh.textureID = ResourceModule::AssetID(textureIdObj.as<std::string>());
                    }
                    else if (textureIdObj.is<ResourceModule::AssetID>())
                    {
                        mesh.textureID = textureIdObj.as<ResourceModule::AssetID>();
                    }
                }
                if (meshTable["vert_shader_id"].valid())
                {
                    sol::object vertShaderIdObj = meshTable["vert_shader_id"];
                    if (vertShaderIdObj.is<std::string>())
                    {
                        mesh.vertShaderID = ResourceModule::AssetID(vertShaderIdObj.as<std::string>());
                    }
                    else if (vertShaderIdObj.is<ResourceModule::AssetID>())
                    {
                        mesh.vertShaderID = vertShaderIdObj.as<ResourceModule::AssetID>();
                    }
                }
                if (meshTable["frag_shader_id"].valid())
                {
                    sol::object fragShaderIdObj = meshTable["frag_shader_id"];
                    if (fragShaderIdObj.is<std::string>())
                    {
                        mesh.fragShaderID = ResourceModule::AssetID(fragShaderIdObj.as<std::string>());
                    }
                    else if (fragShaderIdObj.is<ResourceModule::AssetID>())
                    {
                        mesh.fragShaderID = fragShaderIdObj.as<ResourceModule::AssetID>();
                    }
                }
                e.set<MeshComponent>(mesh);
            }),
        "remove_mesh", [](flecs::entity& e) { RemoveComponentIfPresent<MeshComponent>(e); },
        "has_material", [](const flecs::entity& e) { return e.has<MaterialComponent>(); },
        "get_material",
        [](flecs::entity& e) -> sol::optional<MaterialComponent>
        {
            if (const MaterialComponent* mat = e.get<MaterialComponent>())
                return *mat;
            return sol::nullopt;
        },
        "set_material",
        [](flecs::entity& e, const MaterialComponent& material)
        {
            e.set<MaterialComponent>(material);
        },
        "remove_material", [](flecs::entity& e) { RemoveComponentIfPresent<MaterialComponent>(e); },
        "has_point_light", [](const flecs::entity& e) { return e.has<PointLightComponent>(); },
        "get_point_light",
        [](flecs::entity& e) -> sol::optional<PointLightComponent>
        {
            if (const PointLightComponent* light = e.get<PointLightComponent>())
                return *light;
            return sol::nullopt;
        },
        "set_point_light",
        [](flecs::entity& e, const PointLightComponent& light)
        {
            e.set<PointLightComponent>(light);
        },
        "remove_point_light", [](flecs::entity& e) { RemoveComponentIfPresent<PointLightComponent>(e); },
        "has_directional_light", [](const flecs::entity& e) { return e.has<DirectionalLightComponent>(); },
        "get_directional_light",
        [](flecs::entity& e) -> sol::optional<DirectionalLightComponent>
        {
            if (const DirectionalLightComponent* light = e.get<DirectionalLightComponent>())
                return *light;
            return sol::nullopt;
        },
        "set_directional_light",
        [](flecs::entity& e, const DirectionalLightComponent& light)
        {
            e.set<DirectionalLightComponent>(light);
        },
        "remove_directional_light", [](flecs::entity& e) { RemoveComponentIfPresent<DirectionalLightComponent>(e); },
        "has_rigid_body", [](const flecs::entity& e) { return e.has<PhysicsModule::RigidBodyComponent>(); },
        "get_rigid_body",
        [](flecs::entity& e) -> sol::optional<PhysicsModule::RigidBodyComponent>
        {
            if (const auto* rb = e.get<PhysicsModule::RigidBodyComponent>())
                return *rb;
            return sol::nullopt;
        },
        "set_rigid_body",
        [](flecs::entity& e, const PhysicsModule::RigidBodyComponent& rb)
        {
            e.set<PhysicsModule::RigidBodyComponent>(rb);
        },
        "remove_rigid_body", [](flecs::entity& e) { RemoveComponentIfPresent<PhysicsModule::RigidBodyComponent>(e); },
        "has_collider", [](const flecs::entity& e) { return e.has<PhysicsModule::ColliderComponent>(); },
        "get_collider",
        [](flecs::entity& e) -> sol::optional<PhysicsModule::ColliderComponent>
        {
            if (const auto* collider = e.get<PhysicsModule::ColliderComponent>())
                return *collider;
            return sol::nullopt;
        },
        "set_collider",
        [](flecs::entity& e, const PhysicsModule::ColliderComponent& collider)
        {
            e.set<PhysicsModule::ColliderComponent>(collider);
        },
        "remove_collider", [](flecs::entity& e) { RemoveComponentIfPresent<PhysicsModule::ColliderComponent>(e); },
        "add_component",
        [](flecs::entity& e, const std::string& componentName)
        {
            auto& registry = ECSModule::ComponentRegistry::getInstance();
            return registry.addComponent(e, componentName);
        },
        "remove_component",
        [](flecs::entity& e, const std::string& componentName)
        {
            auto& registry = ECSModule::ComponentRegistry::getInstance();
            return registry.removeComponent(e, componentName);
        },
        "has_editor_only_tag", [](const flecs::entity& e) { return e.has<EditorOnlyTag>(); },
        "add_editor_only_tag", [](flecs::entity& e) { e.add<EditorOnlyTag>(); },
        "remove_editor_only_tag", [](flecs::entity& e) { e.remove<EditorOnlyTag>(); },
        "has_invisible_tag", [](const flecs::entity& e) { return e.has<InvisibleTag>(); },
        "add_invisible_tag", [](flecs::entity& e) { e.add<InvisibleTag>(); },
        "remove_invisible_tag", [](flecs::entity& e) { e.remove<InvisibleTag>(); });
}
} // namespace ScriptModule

