#pragma once

#include <vector>
#include <memory>
#include <unordered_map>

#include <SDL3/SDL.h>
#include "../EventModule/Event.h"


namespace InputModule
{
	class InputModule;
}

namespace Logger
{
	class Logger;
}

namespace WindowModule
{
	/// <summary>
	/// Encapsulates a GLFW window and manages Vulkan-related interactions.
	/// </summary>
	class Window
	{
		/*
		GLFWwindow* m_window;
		*/
		SDL_GLContext m_glContext;
		SDL_Window* m_window; ///< Pointer to the GLFW window instance.
		bool m_framebufferResized = false; ///< Flag indicating if the window was resized.

		Uint64 m_performanceFrequency;
		std::shared_ptr<Logger::Logger> m_logger;
		std::shared_ptr<InputModule::InputModule> m_inputModule;
		
		bool m_shouldClose{false};
	public:
		Event<> OnWindowResized;
		/// <summary>
		/// Constructs a new GLFW window.
		/// </summary>
		/// <param name="width">Width of the window in pixels.</param>
		/// <param name="height">Height of the window in pixels.</param>
		/// <param name="title">Title of the window.</param>
		/// <exception cref="std::runtime_error">Thrown if GLFW initialization fails.</exception>
		Window(std::shared_ptr<Logger::Logger> m_logger, std::shared_ptr<InputModule::InputModule> inputModule, int width, int height, const char* title);

		/// <summary>
		/// Destroys the GLFW window and terminates GLFW.
		/// </summary>
		~Window();

		void registerEventHandlers();
		/*
		/// <summary>
		/// Retrieves the raw GLFW window handle.
		/// </summary>
		/// <returns>Pointer to the GLFW window.</returns>
		GLFWwindow* getGLFWWindow() { return m_window; }
		*/

		SDL_Window* getSDLWindow() {return m_window;}
		SDL_GLContext& getGLContext() {return m_glContext;}

		/// <summary>
		/// Checks if the window was resized.
		/// </summary>
		/// <returns>True if the framebuffer was resized, otherwise false.</returns>
		bool wasResized() const { return m_framebufferResized; }

		/// <summary>
		/// Resets the window resize flag.
		/// </summary>
		void resetResizedFlag() { m_framebufferResized = false; }

		std::pair<int, int> getWindowSize() const;
		/// <summary>
		/// Polls for GLFW window events.
		/// </summary>
		void pollEvents();

		/// <summary>
		/// Checks if the window should close.
		/// </summary>
		/// <returns>True if the window should close, otherwise false.</returns>
		bool shouldClose() const { return m_shouldClose; }

		/// <summary>
		/// Retrieves the current time since GLFW initialization.
		/// </summary>
		/// <returns>Elapsed time in seconds.</returns>
		float currentTimeInSeconds() const;
	};
}
