#pragma once

#include <EngineMinimal.h>

namespace WindowModule
{
    class WindowModule;
}

namespace InputModule
{
    class InputModule;
}

union SDL_Event;

namespace ImGUIModule
{
    class GUIObject;

    /// <summary>
    /// Manages the ImGui user interface system, handling GUI object registration and rendering.
    /// Implements a singleton pattern to ensure a single instance.
    /// </summary>
    class ImGUIModule : public IModule
    {
        InputModule::InputModule* m_inputModule;
        WindowModule::WindowModule* m_windowModule;
        
        std::vector<std::shared_ptr<GUIObject>> m_guiObjects; ///< List of registered GUI objects.

    public:
        void startup() override;
        void shutdown() override;

        /// <summary>
        /// Renders the ImGui user interface.
        /// Calls the render function of all registered GUI objects.
        /// </summary>
        void renderUI(float deltaTime) const;

        std::weak_ptr<GUIObject> addObject(GUIObject* object);
        void removeObject(const std::weak_ptr<GUIObject>& object);
    private:
        void setImGuiStyle() const;
        void onEvent(const SDL_Event& event);
    };
}
