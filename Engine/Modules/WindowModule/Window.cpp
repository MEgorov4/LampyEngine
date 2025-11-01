#include "Window.h"

#include <GL/glew.h>
#include <Modules/InputModule/InputModule.h>
#include <SDL3/SDL_opengl.h>

namespace WindowModule
{

Window::Window(int width, int height, const char *title) : m_inputModule(GCM(InputModule::InputModule))
{
    LT_PROFILE_ZONE("Window::Window");

    LT_LOGI("WindowModule_Window", "Window: Start create window: width = " + std::format("{}", width) + ", height = " +
                                       std::format("{}", height) + ", title = " + std::format("{}", title));

    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS))
    {
        const char *sdlError = SDL_GetError();
        std::string errorMessage = "Failed to initialise SDL: ";
        if (sdlError)
        {
            errorMessage += sdlError;
        }
        else
        {
            errorMessage += "Unknown error.";
        }

        LT_LOGI("WindowModule_Window", errorMessage);

        throw std::runtime_error(errorMessage);
    }

    SDL_GL_ResetAttributes();

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 5);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    m_performanceFrequency = SDL_GetPerformanceFrequency();
    // Создание окна
    m_window = SDL_CreateWindow(title, width, height, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    if (!m_window)
    {
        throw std::runtime_error("Failed to create SDL window");
    }

    // Создание контекста
    m_glContext = SDL_GL_CreateContext(m_window);
    if (!m_glContext)
    {
        const char *error = SDL_GetError();
        throw std::runtime_error("Failed to create OpenGL context: " + std::string(error ? error : "Unknown error"));
    }

    // Установка текущего контекста
    if (!SDL_GL_MakeCurrent(m_window, m_glContext))
    {
        const char *error = SDL_GetError();
        throw std::runtime_error("Failed to make current context: " + std::string(error ? error : "Unknown error"));
    }

    LT_LOGI("WindowModule_Window", "Window created");
}

Window::~Window()
{
    if (m_glContext)
    {
        LT_LOGI("WindowModule_Window", "Destroy context");
        SDL_GL_DestroyContext(m_glContext);
    }
    if (m_window)
    {
        LT_LOGI("WindowModule_Window", "Destroy window");
        SDL_DestroyWindow(m_window);
    }

    LT_LOGI("WindowModule_Window", "SDL quit");
    SDL_Quit();
}

void Window::swapWindow()
{
    LT_PROFILE_ZONE("Window::swapWindow");

    SDL_GL_SwapWindow(m_window);
}

std::pair<int, int> Window::getWindowSize() const
{
    LT_PROFILE_ZONE("Window::getWindowSize");

    std::pair result{0, 0};
    SDL_GetWindowSize(m_window, &result.first, &result.second);
    return result;
}

void Window::pollEvents()
{
    LT_PROFILE_ZONE("Window::pollEvents");

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        m_inputModule->OnEvent(event);
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            m_shouldClose = true;
            break;

        case SDL_EVENT_WINDOW_RESIZED:
            OnWindowResized();
            break;
        case SDL_EVENT_KEY_DOWN:
        case SDL_EVENT_KEY_UP:
            m_inputModule->pushKeyboardEvent(event.key);
            break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN:
        case SDL_EVENT_MOUSE_BUTTON_UP:
            m_inputModule->pushMouseButtonEvent(event.button);
            break;
        case SDL_EVENT_MOUSE_MOTION:
            m_inputModule->pushMouseMotionEvent(event.motion);
            break;
        case SDL_EVENT_MOUSE_WHEEL:
            m_inputModule->pushMouseWheelEvent(event.wheel);
            break;
        default:;
        }
    }
}

float Window::currentTimeInSeconds() const
{
    Uint64 currentPerformanceCounter = SDL_GetPerformanceCounter();
    return static_cast<float>(currentPerformanceCounter) / m_performanceFrequency;
}
} // namespace WindowModule
