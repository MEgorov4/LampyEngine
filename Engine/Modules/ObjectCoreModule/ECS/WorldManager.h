#pragma once
#include <string>
#include <unordered_map>
#include <memory>
#include "EntityWorld.h"

namespace ECSModule
{
    class WorldManager
    {
    public:
        using WorldPtr = std::unique_ptr<EntityWorld>;

        explicit WorldManager(
            ResourceModule::ResourceManager* resources,
            ScriptModule::LuaScriptModule* scripts,
            PhysicsModule::PhysicsModule* physics)
            : m_resources(resources), m_scripts(scripts), m_physics(physics) {}

        EntityWorld& createWorld(const std::string& name);
        void destroyWorld(const std::string& name);

        EntityWorld* getWorld(const std::string& name);
        EntityWorld* getActiveWorld();
        void setActiveWorld(const std::string& name);

        void tickActive(float dt);
        void clear();

        std::vector<std::string> getLoadedWorlds() const;

    private:
        std::unordered_map<std::string, WorldPtr> m_worlds;
        std::string m_activeWorld;

        ResourceModule::ResourceManager* m_resources;
        ScriptModule::LuaScriptModule* m_scripts;
        PhysicsModule::PhysicsModule* m_physics;
    };
}
