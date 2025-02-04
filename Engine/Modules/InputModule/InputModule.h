#pragma once
#include <memory>
#include <functional>
#include "../WindowModule/Window.h"

/// <summary>
/// Manages user input events such as keyboard, mouse movement, and scrolling.
/// Uses function callbacks to handle input from a given window.
/// </summary>
class InputModule
{
    /// <summary>
    /// Callback function for handling keyboard input events.
    /// </summary>
    std::function<void(int key, int scancode, int action, int mods)> m_keyCallback;

    /// <summary>
    /// Callback function for handling mouse cursor movement.
    /// </summary>
    std::function<void(double xpos, double ypos)> m_cursorPositionCallback;

    /// <summary>
    /// Callback function for handling mouse scrolling events.
    /// </summary>
    std::function<void(double xoffset, double yoffset)> m_scrollCallback;

public:
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
    void startUp(Window* window)
    {
        LOG_INFO("InputModule: Startup");

        window->setKeyCallback([](GLFWwindow* win, int key, int scancode, int action, int mods) {
            auto& instance = getInstance();
            if (instance.m_keyCallback) {
                instance.m_keyCallback(key, scancode, action, mods);
            }
            });

        window->setCursorPositionCallback([](GLFWwindow* win, double xpos, double ypos) {
            auto& instance = getInstance();
            if (instance.m_cursorPositionCallback) {
                instance.m_cursorPositionCallback(xpos, ypos);
            }
            });

        window->setScrollCallback([](GLFWwindow* win, double xoffset, double yoffset) {
            auto& instance = getInstance();
            if (instance.m_scrollCallback) {
                instance.m_scrollCallback(xoffset, yoffset);
            }
            });
    }

    /// <summary>
    /// Shuts down the input system and clears registered callbacks.
    /// </summary>
    void shutDown()
    {
        LOG_INFO("InputModule: Shut down");
        m_keyCallback = nullptr;
        m_cursorPositionCallback = nullptr;
        m_scrollCallback = nullptr;
    }

    /// <summary>
    /// Sets a callback function for keyboard input events.
    /// </summary>
    /// <param name="callback">Function to be called when a key event occurs.</param>
    void setKeyCallback(const std::function<void(int key, int scancode, int action, int mods)>& callback)
    {
        m_keyCallback = callback;
    }

    /// <summary>
    /// Sets a callback function for mouse cursor position events.
    /// </summary>
    /// <param name="callback">Function to be called when the mouse moves.</param>
    void setCursorPositionCallback(const std::function<void(double xpos, double ypos)>& callback)
    {
        m_cursorPositionCallback = callback;
    }

    /// <summary>
    /// Sets a callback function for mouse scroll events.
    /// </summary>
    /// <param name="callback">Function to be called when the mouse wheel is scrolled.</param>
    void setScrollCallback(const std::function<void(double xoffset, double yoffset)>& callback)
    {
        m_scrollCallback = callback;
    }
};
