#include "Window.h"
#include <stdexcept>
#include <format>

#include <GL/glew.h>
#include <SDL3/SDL_opengl.h>
#include "../InputModule/InputModule.h"
#include "../LoggerModule/Logger.h"

namespace WindowModule
{
    Window::Window(std::shared_ptr<Logger::Logger> logger, std::shared_ptr<InputModule::InputModule> inputModule,
                   int width, int height, const char* title) : m_logger(logger), m_inputModule(inputModule)
    {
        m_logger->log(Logger::LogVerbosity::Info
                      , "Window: Start create window: width = "
                      + std::format("{}", width)
                      + ", height = "
                      + std::format("{}", height)
                      + ", title = "
                      + std::format("{}", title)
                      , "WindowModule_Window");
        
        if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS))
        {
            const char* sdlError = SDL_GetError(); 
            std::string errorMessage = "Failed to initialise SDL: ";
            if (sdlError) {
                errorMessage += sdlError;
            } else {
                errorMessage += "Unknown error.";
            }
            m_logger->log(Logger::LogVerbosity::Error, errorMessage, "WindowModule_Window");
            
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
            const char* error = SDL_GetError();
            throw std::runtime_error("Failed to create OpenGL context: " + std::string(error ? error : "Unknown error"));
        }

        // Установка текущего контекста
        if (!SDL_GL_MakeCurrent(m_window, m_glContext))
        {
            const char* error = SDL_GetError();
            throw std::runtime_error("Failed to make current context: " + std::string(error ? error : "Unknown error"));
        }

        
        m_logger->log(Logger::LogVerbosity::Info, "Window created", "WindowModule_Window");

        registerEventHandlers();
    }

    Window::~Window()
    {
        if (m_glContext)
        {
            m_logger->log(Logger::LogVerbosity::Info, "Destroy context", "WindowModule_Window");
            SDL_GL_DestroyContext(m_glContext);
        }
        if (m_window)
        {
            m_logger->log(Logger::LogVerbosity::Info, "Destroy window", "WindowModule_Window");
            SDL_DestroyWindow(m_window);
        }

        m_logger->log(Logger::LogVerbosity::Info, "SDL quit", "WindowModule_Window");
        SDL_Quit();
    }

    void Window::registerEventHandlers()
    {
    }


    std::pair<int, int> Window::getWindowSize() const
    {
        std::pair result{0, 0};
        SDL_GetWindowSize(m_window, &result.first, &result.second);
        return result;
    }

    void Window::pollEvents()
    {
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
            default: ;
            }
        }
    }

    float Window::currentTimeInSeconds() const
    {
        Uint64 currentPerformanceCounter = SDL_GetPerformanceCounter();
        return static_cast<float>(currentPerformanceCounter) / m_performanceFrequency;
    }
}
