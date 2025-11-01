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

        // управление мирами
        EntityWorld* getCurrentWorld();
        EntityWorld& createWorld(const std::string& name);
        void openBasicWorld();
        void openWorldByResource(const std::string& guid);
        void closeWorld(const std::string& name);
        void simulate(bool state);
        bool isSimulate() const { return m_inSimulate; }
    private:
        std::unique_ptr<WorldManager> m_worldManager;
        ModuleEventBinder m_binder;          ///< подключение к глобальной шине событий
        bool m_inSimulate = false;
    };
}