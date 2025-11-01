#pragma once
#include "Components/ECSComponents.h"
#include "flecs.h"

#include <EngineMinimal.h>

namespace ResourceModule
{
class ResourceManager;
}
namespace ScriptModule
{
class LuaScriptModule;
}
namespace PhysicsModule
{
class PhysicsModule;
}

class EntityWorld
{
  public:
    EntityWorld(ResourceModule::ResourceManager* resources, ScriptModule::LuaScriptModule* scripts,
                PhysicsModule::PhysicsModule* physics);

    void init();
    void reset();
    void tick(float dt);

    std::string serialize();
    void deserialize(const std::string& jsonData);

    flecs::world& get() noexcept
    {
        return m_world;
    }

  private:
    void registerTypes();
    void registerComponents();
    void registerObservers();

  private:
    flecs::world m_world;
    ResourceModule::ResourceManager* m_resources;
    ScriptModule::LuaScriptModule* m_scripts;
    PhysicsModule::PhysicsModule* m_physics;
};
