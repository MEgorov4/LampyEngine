#pragma once

#include <memory>
#include <stdexcept>
#include <cassert>

#include "Window.h"
#include "../LoggerModule/Logger.h"

class WindowModule
{
    std::unique_ptr<Window> m_window;
public:
    WindowModule() {}
    ~WindowModule() {}

    static WindowModule& getInstance()
    {
        static WindowModule WindowModule;
        return WindowModule;
    }

    void startUp(int width, int height, const char* title)
    {
        LOG_INFO("WindowModule: Startup");
        m_window = std::make_unique<Window>(width, height, title);
    }

    void shutDown()
    {
        LOG_INFO("WindowModule: Shut down");
        m_window.reset();
    }
    

    Window* getWindow()
    {
        Window* window = m_window.get();
        assert(window);
        return window;
    }
};
