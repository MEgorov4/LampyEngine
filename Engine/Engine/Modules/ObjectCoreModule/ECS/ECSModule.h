#pragma once
#include <EngineMinimal.h>
#include "WorldManager.h"

namespace ECSModule
{
    class ECSModule : public IModule
    {
    public:
        void startup() override;
        void shutdown() override;
        void ecsTick(float dt);

        EntityWorld* getCurrentWorld();
        EntityWorld& createWorld(const std::string& name);
        void openBasicWorld();
        void openWorldByResource(const std::string& guid);
        void closeWorld(const std::string& name);
        void simulate(bool state);
        bool isSimulate() const { return m_inSimulate; }
    private:
        void registerComponents();
        
        void emitRenderFrameData();
        
        std::unique_ptr<WorldManager> m_worldManager;
        ModuleEventBinder m_binder;
        bool m_inSimulate = false;
        std::string m_simulationSnapshot; // Snapshot of world state before simulation
    };
}