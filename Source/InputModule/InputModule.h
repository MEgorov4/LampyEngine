#pragma once
#include <memory>
#include <functional>

#include "../WindowModule/Window.h"
class InputModule
{
    std::function<void(int key, int scancode, int action, int mods)> keyCallback;
    std::function<void(double xpos, double ypos)> cursorPositionCallback;
    std::function<void(double xoffset, double yoffset)> scrollCallback;
public:
	InputModule() {}
	~InputModule() {}
	static InputModule& getInstance()
	{
		static InputModule InputModule;
		return InputModule;
	}

	void startUp(Window* window)
	{
        window->setKeyCallback([](GLFWwindow* win, int key, int scancode, int action, int mods) {
            auto& instance = getInstance();
            if (instance.keyCallback) {
                instance.keyCallback(key, scancode, action, mods);
            }
            });

        window->setCursorPositionCallback([](GLFWwindow* win, double xpos, double ypos) {
            auto& instance = getInstance();
            if (instance.cursorPositionCallback) {
                instance.cursorPositionCallback(xpos, ypos);
            }
            });

        window->setScrollCallback([](GLFWwindow* win, double xoffset, double yoffset) {
            auto& instance = getInstance();
            if (instance.scrollCallback) {
                instance.scrollCallback(xoffset, yoffset);
            }
            });
	}

	void shutDown()
	{
        keyCallback = nullptr;
        cursorPositionCallback = nullptr;
        scrollCallback = nullptr;
	}

    void setKeyCallback(const std::function<void(int key, int scancode, int action, int mods)>& callback)
    {
        keyCallback = callback;
    }

    void setCursorPositionCallback(const std::function<void(double xpos, double ypos)>& callback)
    {
        cursorPositionCallback = callback;
    }

    void setScrollCallback(const std::function<void(double xoffset, double yoffset)>& callback)
    {
        scrollCallback = callback;
    }
};