#include "InputModule.h"

#include "../LoggerModule/Logger.h"
#include "../WindowModule/Window.h"

void InputModule::startup(Window* window)
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


void InputModule::shutDown()
{
    LOG_INFO("InputModule: Shut down");
}
