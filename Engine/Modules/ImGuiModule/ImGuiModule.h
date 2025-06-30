#pragma once
#include <vector>
#include <memory>

#include "../../EngineContext/IModule.h"
#include "../../EngineContext/ModuleRegistry.h"

namespace Logger
{
    class Logger;
}

namespace ImGuiModule
{
    class GUIObject;

    /// <summary>
    /// Manages the ImGui user interface system, handling GUI object registration and rendering.
    /// Implements a singleton pattern to ensure a single instance.
    /// </summary>
    class ImGuiModule : public IModule
    {
        std::shared_ptr<Logger::Logger> m_logger;
        std::vector<std::shared_ptr<GUIObject>> m_guiObjects; ///< List of registered GUI objects.

    public:
        void startup(const ModuleRegistry& registry) override;
        void shutdown() override;

        void setImGuiStyle();

        /// <summary>
        /// Renders the ImGui user interface.
        /// Calls the render function of all registered GUI objects.
        /// </summary>
        void renderUI() const;

        std::weak_ptr<GUIObject> addObject(GUIObject* object);
        void removeObject(const std::weak_ptr<GUIObject>& object);
    };
}
