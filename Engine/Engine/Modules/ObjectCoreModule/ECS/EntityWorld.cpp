#include "EntityWorld.h"

#include "Additionals/ECSSerializeTypes.h"
#include "Modules/ResourceModule/ResourceManager.h"
#include "Systems/ECSLuaScriptsSystem.h"
#include "Systems/ECSPhysicsSystem.h"

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
    return m_world.to_json().c_str();
}

void EntityWorld::deserialize(const std::string& json)
{
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
}

void EntityWorld::registerComponents()
{
}

void EntityWorld::registerObservers()
{
}
