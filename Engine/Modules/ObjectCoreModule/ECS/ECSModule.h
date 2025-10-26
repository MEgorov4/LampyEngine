#pragma once
#include <memory>
#include <optional>
#include <string>
#include <flecs.h>

#include "../../EventModule/Event.h"

#include "../../../EngineContext/IModule.h"


namespace PhysicsModule
{
    class PhysicsModule;
}

namespace ScriptModule
{
    class LuaScriptModule;
}

namespace FilesystemModule
{
    class FilesystemModule;
}

namespace ProjectModule
{
    class ProjectModule;
}

namespace ResourceModule
{
    class ResourceManager;
}

namespace Logger
{
    class Logger;
}

namespace ECSModule
{
    class ECSModule : public IModule
    {
        Logger::Logger* m_logger;
        FilesystemModule::FilesystemModule* m_filesystemModule;
        ProjectModule::ProjectModule* m_projectModule;
        ResourceModule::ResourceManager* m_resourceManager;
        ScriptModule::LuaScriptModule* m_luaScriptModule;
        PhysicsModule::PhysicsModule* m_physicsModule;

        bool m_inSimulate = false;

        flecs::world m_world;
        std::string m_worldTemp;
        std::vector<std::pair<flecs::id_t, std::string>> m_registeredComponents;

    public:
        void startup() override;
        void shutdown() override;

    public:
        /// <summary>
        /// Basic interface to process world
        /// </summary>
        /// <param name="deltaTime">diff last and curr frame</param>
        void ecsTick(float deltaTime);
        /// <summary>
        /// Reset current world 
        /// and open world from json data
        /// </summary>
        /// <param name="worldData"> - Json data</param>
        void openWorld(const std::string& worldData);

        /// <summary>
        /// Open basic world template
        /// </summary>
        void openBasicWorld();

        /// <summary>
        /// Toogle world simulation and systems tick
        /// </summary>
        /// <param name="state"></param>
        void simulate(bool state);
        
        /// <summary>
        /// Check current world and systems simulation state
        /// </summary>
        /// <returns>Simulation state on/off(true/false)</returns>
        bool isSimulate() const  { return m_inSimulate; }
        std::string getCurrentWorldData() { return m_world.to_json().c_str(); };
        flecs::world& getCurrentWorld() { return m_world; };
        std::vector<std::pair<flecs::id_t, std::string>>& getRegisteredComponents() { return m_registeredComponents; };

        Event<> OnLoadInitialWorldState;
        Event<> OnComponentsChanged;

    private:
        /// <summary>
        /// Clear world and rebind systems
        /// </summary>
        void resetWorld();

        template <typename T>
        void registerComponent(const std::string& name);
        void registerComponents();
        void registerTypes();
        void registerObservers();
    };
}
