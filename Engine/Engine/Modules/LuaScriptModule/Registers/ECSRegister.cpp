#include "ECSRegister.h"

#include "../LuaScriptModule.h"

#include "../../ObjectCoreModule/ECS/Components/ECSComponents.h"
#include "../../ObjectCoreModule/ECS/ECSModule.h"
#include "../../ObjectCoreModule/ECS/ComponentRegistry.h"
#include "../../ObjectCoreModule/ECS/EntityWorld.h"
#include <Modules/PhysicsModule/Components/RigidBodyComponent.h>
#include <Modules/PhysicsModule/Components/ColliderComponent.h>
#include <Modules/PhysicsModule/Utils/PhysicsTypes.h>

#include <Modules/ResourceModule/Asset/AssetID.h>

#include <glm/glm.hpp>
#include <flecs.h>
#include <sol/sol.hpp>
#include <optional>

namespace ScriptModule
{
namespace
{
using ResourceModule::AssetID;
using namespace ECSModule;
using namespace PhysicsModule;

template <typename Comp>
void removeComponentIfPresent(flecs::entity& e)
{
    if (e.has<Comp>())
    {
        e.remove<Comp>();
    }
}
} // namespace

void ECSRegister::registerTypes(LuaScriptModule& module, sol::state& state)
{
    state.set_function(
        "GetCurrentWorld",
        [&module]() -> flecs::world*
        {
            auto* ecsModule = module.getECSModule();
            if (!ecsModule)
                return nullptr;
            EntityWorld* worldWrapper = ecsModule->getCurrentWorld();
            if (!worldWrapper)
                return nullptr;
            return &worldWrapper->get();
        });
    state.new_usertype<AssetID>(
        "AssetIDType",
        "str", &AssetID::str,
        "empty", &AssetID::empty,
        "__tostring", [](const AssetID& id) { return id.str(); });
    
    sol::table assetIDTable = state.create_table();
    assetIDTable["new"] = sol::overload(
        []() -> AssetID { return AssetID(); },
        [](const std::string& str) -> AssetID { return AssetID(str); }
    );
    state["AssetID"] = assetIDTable;

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
    
    state.set_function("RotationComponent", []() -> RotationComponent { return RotationComponent(); });

    state.new_usertype<ScaleComponent>(
        "ScaleComponent",
        sol::constructors<ScaleComponent()>(),
        "x", &ScaleComponent::x,
        "y", &ScaleComponent::y,
        "z", &ScaleComponent::z,
        "to_vec3", [](const ScaleComponent& self) { return self.toGLMVec(); },
        "from_vec3", [](ScaleComponent& self, const glm::vec3& v) { self.fromGMLVec(v); });

    state.new_usertype<CameraComponent>(
        "CameraComponent",
        sol::constructors<CameraComponent()>(),
        "fov", &CameraComponent::fov,
        "aspect", &CameraComponent::aspect,
        "near_clip", &CameraComponent::nearClip,
        "far_clip", &CameraComponent::farClip,
        "is_viewport_camera", &CameraComponent::isViewportCamera);

    state.new_usertype<MeshComponent>(
        "MeshComponent",
        sol::constructors<MeshComponent()>(),
        "mesh_id", &MeshComponent::meshID,
        "texture_id", &MeshComponent::textureID,
        "vert_shader_id", &MeshComponent::vertShaderID,
        "frag_shader_id", &MeshComponent::fragShaderID);

    state.new_usertype<PointLightComponent>(
        "PointLightComponentType",
        "inner_radius", &PointLightComponent::innerRadius,
        "outer_radius", &PointLightComponent::outerRadius,
        "intensity", &PointLightComponent::intencity,
        "color", &PointLightComponent::color);
    
    sol::table pointLightComponentTable = state.create_table();
    pointLightComponentTable["new"] = []() -> PointLightComponent { return PointLightComponent(); };
    state["PointLightComponent"] = pointLightComponentTable;

    state.new_usertype<DirectionalLightComponent>(
        "DirectionalLightComponent",
        sol::constructors<DirectionalLightComponent()>(),
        "intensity", &DirectionalLightComponent::intencity);

    state.new_usertype<MaterialComponent>(
        "MaterialComponentType",
        "material_id", &MaterialComponent::materialID);
    
    sol::table materialComponentTable = state.create_table();
    materialComponentTable["new"] = []() -> MaterialComponent { return MaterialComponent(); };
    state["MaterialComponent"] = materialComponentTable;

    state.new_usertype<EditorOnlyTag>("EditorOnlyTag");
    state.new_usertype<InvisibleTag>("InvisibleTag");

    // Physics Module Components
    // PhysicsShapeType enum
    state.new_enum("PhysicsShapeType",
        "Box", PhysicsShapeType::Box,
        "Sphere", PhysicsShapeType::Sphere,
        "Capsule", PhysicsShapeType::Capsule,
        "Cylinder", PhysicsShapeType::Cylinder,
        "Mesh", PhysicsShapeType::Mesh,
        "ConvexHull", PhysicsShapeType::ConvexHull
    );

    // PhysicsShapeDesc
    state.new_usertype<PhysicsShapeDesc>(
        "PhysicsShapeDesc",
        sol::constructors<PhysicsShapeDesc()>(),
        "type", &PhysicsShapeDesc::type,
        "size", &PhysicsShapeDesc::size,
        "radius", &PhysicsShapeDesc::radius,
        "height", &PhysicsShapeDesc::height
    );
    
    sol::table physicsShapeDescTable = state.create_table();
    physicsShapeDescTable["new"] = []() -> PhysicsShapeDesc { return PhysicsShapeDesc(); };
    state["PhysicsShapeDesc"] = physicsShapeDescTable;

    // RigidBodyComponent
    state.new_usertype<RigidBodyComponent>(
        "RigidBodyComponent",
        sol::constructors<RigidBodyComponent()>(),
        "mass", &RigidBodyComponent::mass,
        "isStatic", &RigidBodyComponent::isStatic,
        "isKinematic", &RigidBodyComponent::isKinematic,
        "needsCreation", &RigidBodyComponent::needsCreation
    );
    
    sol::table rigidBodyComponentTable = state.create_table();
    rigidBodyComponentTable["new"] = []() -> RigidBodyComponent { return RigidBodyComponent(); };
    state["RigidBodyComponent"] = rigidBodyComponentTable;

    // ColliderComponent
    state.new_usertype<ColliderComponent>(
        "ColliderComponent",
        sol::constructors<ColliderComponent()>(),
        "shapeDesc", &ColliderComponent::shapeDesc,
        "isTrigger", &ColliderComponent::isTrigger,
        "needsCreation", &ColliderComponent::needsCreation
    );
    
    sol::table colliderComponentTable = state.create_table();
    colliderComponentTable["new"] = []() -> ColliderComponent { return ColliderComponent(); };
    state["ColliderComponent"] = colliderComponentTable;

    state.new_usertype<flecs::world>(
        "World",
        "entity",
        [](flecs::world& w, const std::string& name) -> flecs::entity
        {
            if (name.empty())
                return w.entity();
            return w.entity(name.c_str());
        },
        "create",
        [](flecs::world& w, const std::optional<std::string>& name) -> flecs::entity
        {
            if (name && !name->empty())
                return w.entity(name->c_str());
            return w.entity();
        },
        "find",
        [](flecs::world& w, const std::string& name) -> flecs::entity
        {
            return w.lookup(name.c_str());
        },
        "destroy",
        [](flecs::world&, flecs::entity entity)
        {
            if (entity.is_alive())
                entity.destruct();
        });

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
        "add_position",
        [](flecs::entity& e, const glm::vec3& value)
        {
            PositionComponent pos{};
            pos.fromGLMVec(value);
            e.set<PositionComponent>(pos);
        },
        "remove_position", [](flecs::entity& e) { removeComponentIfPresent<PositionComponent>(e); },
        "has_position", [](const flecs::entity& e) { return e.has<PositionComponent>(); },
        "get_position",
        [](flecs::entity& e) -> sol::optional<glm::vec3>
        {
            if (const PositionComponent* pos = e.get<PositionComponent>())
                return pos->toGLMVec();
            return sol::nullopt;
        },
        "set_position",
        [](flecs::entity& e, const glm::vec3& value)
        {
            if (auto* pos = e.get_mut<PositionComponent>())
            {
                pos->fromGLMVec(value);
            }
            else
            {
                PositionComponent newPos{};
                newPos.fromGLMVec(value);
                e.set<PositionComponent>(newPos);
            }
        },
        "has_rotation", [](const flecs::entity& e) { return e.has<RotationComponent>(); },
        "get_rotation",
        [](flecs::entity& e) -> sol::optional<RotationComponent>
        {
            if (const RotationComponent* rot = e.get<RotationComponent>())
                return *rot;
            return sol::nullopt;
        },
        "set_rotation",
        [](flecs::entity& e, const RotationComponent& value)
        {
            e.set<RotationComponent>(value);
        },
        "remove_rotation", [](flecs::entity& e) { removeComponentIfPresent<RotationComponent>(e); },
        "has_scale", [](const flecs::entity& e) { return e.has<ScaleComponent>(); },
        "get_scale",
        [](flecs::entity& e) -> sol::optional<glm::vec3>
        {
            if (const ScaleComponent* scale = e.get<ScaleComponent>())
                return scale->toGLMVec();
            return sol::nullopt;
        },
        "set_scale",
        [](flecs::entity& e, const glm::vec3& value)
        {
            if (auto* scale = e.get_mut<ScaleComponent>())
            {
                scale->fromGMLVec(value);
            }
            else
            {
                ScaleComponent sc{};
                sc.fromGMLVec(value);
                e.set<ScaleComponent>(sc);
            }
        },
        "remove_scale", [](flecs::entity& e) { removeComponentIfPresent<ScaleComponent>(e); },
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
        "remove_camera", [](flecs::entity& e) { removeComponentIfPresent<CameraComponent>(e); },
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
                        mesh.meshID = AssetID(meshIdObj.as<std::string>());
                    }
                    else if (meshIdObj.is<AssetID>())
                    {
                        mesh.meshID = meshIdObj.as<AssetID>();
                    }
                }
                if (meshTable["texture_id"].valid())
                {
                    sol::object textureIdObj = meshTable["texture_id"];
                    if (textureIdObj.is<std::string>())
                    {
                        mesh.textureID = AssetID(textureIdObj.as<std::string>());
                    }
                    else if (textureIdObj.is<AssetID>())
                    {
                        mesh.textureID = textureIdObj.as<AssetID>();
                    }
                }
                if (meshTable["vert_shader_id"].valid())
                {
                    sol::object vertShaderIdObj = meshTable["vert_shader_id"];
                    if (vertShaderIdObj.is<std::string>())
                    {
                        mesh.vertShaderID = AssetID(vertShaderIdObj.as<std::string>());
                    }
                    else if (vertShaderIdObj.is<AssetID>())
                    {
                        mesh.vertShaderID = vertShaderIdObj.as<AssetID>();
                    }
                }
                if (meshTable["frag_shader_id"].valid())
                {
                    sol::object fragShaderIdObj = meshTable["frag_shader_id"];
                    if (fragShaderIdObj.is<std::string>())
                    {
                        mesh.fragShaderID = AssetID(fragShaderIdObj.as<std::string>());
                    }
                    else if (fragShaderIdObj.is<AssetID>())
                    {
                        mesh.fragShaderID = fragShaderIdObj.as<AssetID>();
                    }
                }
                e.set<MeshComponent>(mesh);
            }
        ),
        "remove_mesh", [](flecs::entity& e) { removeComponentIfPresent<MeshComponent>(e); },
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
        "remove_material", [](flecs::entity& e) { removeComponentIfPresent<MaterialComponent>(e); },
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
        "remove_point_light", [](flecs::entity& e) { removeComponentIfPresent<PointLightComponent>(e); },
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
        "remove_directional_light", [](flecs::entity& e) { removeComponentIfPresent<DirectionalLightComponent>(e); },
        // Physics components
        "has_rigid_body", [](const flecs::entity& e) { return e.has<RigidBodyComponent>(); },
        "get_rigid_body",
        [](flecs::entity& e) -> sol::optional<RigidBodyComponent>
        {
            if (const RigidBodyComponent* rb = e.get<RigidBodyComponent>())
                return *rb;
            return sol::nullopt;
        },
        "set_rigid_body",
        [](flecs::entity& e, const RigidBodyComponent& rb)
        {
            e.set<RigidBodyComponent>(rb);
        },
        "remove_rigid_body", [](flecs::entity& e) { removeComponentIfPresent<RigidBodyComponent>(e); },
        "has_collider", [](const flecs::entity& e) { return e.has<ColliderComponent>(); },
        "get_collider",
        [](flecs::entity& e) -> sol::optional<ColliderComponent>
        {
            if (const ColliderComponent* collider = e.get<ColliderComponent>())
                return *collider;
            return sol::nullopt;
        },
        "set_collider",
        [](flecs::entity& e, const ColliderComponent& collider)
        {
            e.set<ColliderComponent>(collider);
        },
        "remove_collider", [](flecs::entity& e) { removeComponentIfPresent<ColliderComponent>(e); },
        // Generic component add/remove using ComponentRegistry
        "add_component",
        [](flecs::entity& e, const std::string& componentName)
        {
            auto& registry = ComponentRegistry::getInstance();
            return registry.addComponent(e, componentName);
        },
        "remove_component",
        [](flecs::entity& e, const std::string& componentName)
        {
            auto& registry = ComponentRegistry::getInstance();
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

