#pragma once
#include <memory>
#include <optional>
#include <string>
#include <flecs.h>

#include "../../EventModule/Event.h"

#include "../../../EngineContext/IModule.h"
#include "../../../EngineContext/ModuleRegistry.h"


class PhysicsModule;

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
        std::shared_ptr<Logger::Logger> m_logger;
        std::shared_ptr<FilesystemModule::FilesystemModule> m_filesystemModule;
        std::shared_ptr<ProjectModule::ProjectModule> m_projectModule;
        std::shared_ptr<ResourceModule::ResourceManager> m_resourceManager;
        std::shared_ptr<ScriptModule::LuaScriptModule> m_luaScriptModule;
        std::shared_ptr<PhysicsModule> m_physicsModule;
        
        bool m_tickEnabled = false;
        std::string m_currentWorldFile;
        std::string m_currentWorldData;
        flecs::world m_world;
        std::vector<std::pair<flecs::id_t, std::string>> m_registeredComponents;
    public:

        void startup(const ModuleRegistry& registry) override;
        void shutdown() override;
        
        void fillDefaultWorld();
        void loadWorldFromFile(const std::string& path);
        void setCurrentWorldPath(const std::string& path);
        void saveCurrentWorld();
        bool isWorldSetted();
        void clearWorld();

        void startSystems();
        void stopSystems();

        bool getTickEnabled() { return m_tickEnabled; }
        flecs::world& getCurrentWorld() { return m_world; };
        std::vector<std::pair<flecs::id_t, std::string>>& getRegisteredComponents() { return m_registeredComponents; };

        void ecsTick(float deltaTime);


        Event<> OnLoadInitialWorldState;
        Event<> OnComponentsChanged;

    private:
        template <typename T>
        void registerComponent(const std::string& name);
        void registerComponents();
        void registerObservers();
    };
}
