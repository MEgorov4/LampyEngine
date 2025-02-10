#pragma once
#include <memory>
#include <functional>
#include "../WindowModule/Window.h"
#include "../EventModule/Event.h"
/// <summary>
/// Manages user input events such as keyboard, mouse movement, and scrolling.
/// Uses function callbacks to handle input from a given window.
/// </summary>
class InputModule
{
public:
    Event<int, int, int, int> OnKeyAction;

    Event<double, double> OnScrollAction;

    Event<double, double> OnMousePosAction;

    /// <summary>
    /// Constructs an empty InputModule.
    /// </summary>
    InputModule() {}

    /// <summary>
    /// Destroys the InputModule.
    /// </summary>
    ~InputModule() {}

    /// <summary>
    /// Retrieves the singleton instance of the InputModule.
    /// </summary>
    /// <returns>Reference to the singleton InputModule instance.</returns>
    static InputModule& getInstance()
    {
        static InputModule InputModule;
        return InputModule;
    }

    /// <summary>
    /// Initializes the input system and registers callbacks for input events.
    /// </summary>
    /// <param name="window">Pointer to the window where input will be captured.</param>
    void startup(Window* window)
    {
        LOG_INFO("InputModule: Startup");

        window->setKeyCallback([](GLFWwindow* win, int key, int scancode, int action, int mods) {
            auto& instance = getInstance();
            instance.OnKeyAction(key, scancode, action, mods);
            });

        window->setCursorPositionCallback([](GLFWwindow* win, double xpos, double ypos) {
            auto& instance = getInstance();
            instance.OnMousePosAction(xpos, ypos);
            });

        window->setScrollCallback([](GLFWwindow* win, double xoffset, double yoffset) {
            auto& instance = getInstance();
            instance.OnScrollAction(xoffset, yoffset);
            });
    }

    /// <summary>
    /// Shuts down the input system and clears registered callbacks.
    /// </summary>
    void shutDown()
    {
        LOG_INFO("InputModule: Shut down");
    }
};
