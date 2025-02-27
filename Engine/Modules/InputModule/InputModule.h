#pragma once
#include <memory>
#include <functional>
#include "../EventModule/Event.h"
class Window;
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
    void startup(Window* window);

    /// <summary>
    /// Shuts down the input system and clears registered callbacks.
    /// </summary>
    void shutDown();
};
