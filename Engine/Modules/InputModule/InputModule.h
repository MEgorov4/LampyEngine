#pragma once

#include <memory>

#include "../../EngineContext/IModule.h"
#include "../../EngineContext/ModuleRegistry.h"

#include "../EventModule/Event.h"

namespace Logger
{
    class Logger;
}

namespace WindowModule
{
    class WindowModule;
}

namespace InputModule
{
    /// <summary>
    /// Manages user input events such as keyboard, mouse movement, and scrolling.
    /// Uses function callbacks to handle input from a given window.
    /// </summary>
    class InputModule : public IModule
    {
        std::shared_ptr<Logger::Logger> m_logger;
        std::shared_ptr<WindowModule::WindowModule> m_windowModule;
    public:
        Event<int, int, int, int> OnKeyAction;

        Event<double, double> OnScrollAction;

        Event<double, double> OnMousePosAction;

        /// <summary>
        /// Initializes the input system and registers callbacks for input events.
        /// </summary>
        /// <param name="window">Pointer to the window where input will be captured.</param>
        void startup(const ModuleRegistry& registry) override;

        /// <summary>
        /// Shuts down the input system and clears registered callbacks.
        /// </summary>
        void shutdown() override;

    private:
        InputModule& getInstance();
    };
}
