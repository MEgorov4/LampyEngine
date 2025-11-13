#include "EntityWorld.h"

#include "Additionals/ECSSerializeTypes.h"
#include "Modules/ResourceModule/ResourceManager.h"
#include "Systems/ECSLuaScriptsSystem.h"
#include "Systems/ECSPhysicsSystem.h"
#include <Modules/PhysicsModule/Systems/SyncToPhysicsSystem.h>
#include <Modules/PhysicsModule/Systems/SyncFromPhysicsSystem.h>
#include <cstring>
#include <nlohmann/json.hpp>

EntityWorld::EntityWorld(ResourceModule::ResourceManager* r, ScriptModule::LuaScriptModule* s,
                         PhysicsModule::PhysicsModule* p) : m_resources(r), m_scripts(s), m_physics(p)
{
}

void EntityWorld::init()
{
    m_world.import <flecs::stats>();
    registerTypes();
    registerComponents();
    registerObservers();

    ECSluaScriptsSystem::getInstance().registerSystem(m_world, m_scripts);
    ECSPhysicsSystem::getInstance().registerSystem(m_world);
    
    // Register PhysicsModule systems
    // SyncToPhysics runs in OnUpdate, then physics step is called from PhysicsModule::tick(),
    // then SyncFromPhysics runs in OnUpdate (on next frame or after step)
    PhysicsModule::SyncToPhysicsSystem::Register(m_world);
    PhysicsModule::SyncFromPhysicsSystem::Register(m_world);
    // PhysicsStepSystem is not needed - step is called from PhysicsModule::tick()
       
    using namespace ResourceModule;
    auto& viewport_camera = m_world.entity("ViewportCamera")
                                .set<PositionComponent>({0.f, 2.f, 5.f})
                                .set<RotationComponent>({-15.f, 0.f, 0.f})
                                .set<CameraComponent>({75.0f, 16.0f / 9.0f, 0.1f, 100.0f, true});
}

void EntityWorld::reset()
{
    m_world.reset();
    init();
}

void EntityWorld::tick(float dt)
{
    ZoneScopedN("EntityWorld::tick");
    m_world.progress(dt);
}

std::string EntityWorld::serialize()
{
    // Use flecs built-in serialization via query
    // Query all entities and serialize them using flecs entity serialization
    // This ensures components are properly serialized using flecs built-in methods
    
    std::string result = "{\"results\":[";
    bool first = true;
    
    // Query for entities with PositionComponent (most common)
    auto queryPos = m_world.query<PositionComponent>();
    queryPos.each([&result, &first](flecs::entity e, PositionComponent&) {
        if (e.is_valid())
        {
            const char* name = e.name();
            bool isSystemEntity = e.has(flecs::System);
            bool isFlecsInternal = (name && std::strncmp(name, "flecs.", 6) == 0);
            
            if (!isSystemEntity && !isFlecsInternal)
            {
                if (!first)
                    result += ",";
                first = false;
                // Use flecs built-in entity serialization
                result += e.to_json().c_str();
            }
        }
    });
    
    // Also query for entities with RotationComponent but no PositionComponent
    auto queryRot = m_world.query<RotationComponent>();
    queryRot.each([&result, &first](flecs::entity e, RotationComponent&) {
        if (e.has<PositionComponent>())
            return; // Already serialized
        
        if (e.is_valid())
        {
            const char* name = e.name();
            bool isSystemEntity = e.has(flecs::System);
            bool isFlecsInternal = (name && std::strncmp(name, "flecs.", 6) == 0);
            
            if (!isSystemEntity && !isFlecsInternal)
            {
                if (!first)
                    result += ",";
                first = false;
                result += e.to_json().c_str();
            }
        }
    });
    
    result += "]}";
    return result;
}

void EntityWorld::deserialize(const std::string& json)
{
    // Clear all existing user entities before deserializing
    // Collect all user entities first to avoid iterator invalidation
    std::vector<flecs::entity> entitiesToDestroy;
    m_world.each([&entitiesToDestroy](flecs::entity e) {
        if (e.is_valid())
        {
            const char* name = e.name();
            bool isSystemEntity = e.has(flecs::System);
            bool isFlecsInternal = (name && std::strncmp(name, "flecs.", 6) == 0);
            
            if (!isSystemEntity && !isFlecsInternal)
            {
                entitiesToDestroy.push_back(e);
            }
        }
    });
    
    // Destroy all collected entities
    for (auto& e : entitiesToDestroy)
    {
        if (e.is_valid())
        {
            e.destruct();
        }
    }
    
    // Use flecs built-in deserialization
    // This should restore all entities with their components
    m_world.from_json(json.c_str());
}

void EntityWorld::registerTypes()
{
    m_world.component<std::string>()
        .opaque(flecs::String)
        .serialize(ECSModule::Utils::string_serialize)
        .assign_string(ECSModule::Utils::string_assign_string)
        .assign_null(ECSModule::Utils::string_assign_null);

    m_world.component<ResourceModule::AssetID>()
        .opaque(flecs::String)
        .serialize(ECSModule::Utils::AssetID_serialize)
        .assign_string(ECSModule::Utils::AssetID_assign_string)
        .assign_null(ECSModule::Utils::AssetID_assign_null);
    
    // Register glm::vec3 for serialization (used in PointLightComponent)
    // Need to register with members for proper serialization
    m_world.component<glm::vec3>()
        .member<float>("x")
        .member<float>("y")
        .member<float>("z");
}

void EntityWorld::registerComponents()
{
    // Register all ECS components with reflection data for serialization
    // This is required for flecs to properly serialize/deserialize components
    
    // PositionComponent
    m_world.component<PositionComponent>()
        .member<float>("x")
        .member<float>("y")
        .member<float>("z");
    
    // RotationComponent
    m_world.component<RotationComponent>()
        .member<float>("x")
        .member<float>("y")
        .member<float>("z")
        .member<float>("qx")
        .member<float>("qy")
        .member<float>("qz")
        .member<float>("qw");
    
    // ScaleComponent
    m_world.component<ScaleComponent>()
        .member<float>("x")
        .member<float>("y")
        .member<float>("z");
    
    // CameraComponent
    m_world.component<CameraComponent>()
        .member<float>("fov")
        .member<float>("aspect")
        .member<float>("nearClip")
        .member<float>("farClip")
        .member<bool>("isViewportCamera");
    
    // MeshComponent - uses AssetID which is already registered as opaque String
    m_world.component<MeshComponent>()
        .member<ResourceModule::AssetID>("meshID")
        .member<ResourceModule::AssetID>("textureID")
        .member<ResourceModule::AssetID>("vertShaderID")
        .member<ResourceModule::AssetID>("fragShaderID");
    
    // MaterialComponent
    m_world.component<MaterialComponent>()
        .member<ResourceModule::AssetID>("materialID");
    
    // PointLightComponent
    m_world.component<PointLightComponent>()
        .member<float>("innerRadius")
        .member<float>("outerRadius")
        .member<float>("intencity")
        .member<glm::vec3>("color");
    
    // DirectionalLightComponent
    m_world.component<DirectionalLightComponent>()
        .member<float>("intencity");
    
    // ScriptComponent
    m_world.component<ScriptComponent>()
        .member<ResourceModule::AssetID>("scriptID");
    
    // Register tags (no members needed)
    m_world.component<EditorOnlyTag>();
    m_world.component<InvisibleTag>();
}

void EntityWorld::registerObservers()
{
}
