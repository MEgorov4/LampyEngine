#pragma once

#include "GL/glew.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>
#include <memory>

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
		GLFWwindow* m_window; ///< Pointer to the GLFW window instance.
		bool m_framebufferResized = false; ///< Flag indicating if the window was resized.

		std::shared_ptr<Logger::Logger> m_logger;
	public:
		/// <summary>
		/// Constructs a new GLFW window.
		/// </summary>
		/// <param name="width">Width of the window in pixels.</param>
		/// <param name="height">Height of the window in pixels.</param>
		/// <param name="title">Title of the window.</param>
		/// <exception cref="std::runtime_error">Thrown if GLFW initialization fails.</exception>
		Window(std::shared_ptr<Logger::Logger> m_logger, int width, int height, const char* title);

		/// <summary>
		/// Destroys the GLFW window and terminates GLFW.
		/// </summary>
		~Window();

		/// <summary>
		/// Checks if the GLFW window is valid.
		/// </summary>
		/// <returns>True if the window is successfully created, otherwise false.</returns>
		bool glfwWindowIsValid() const { return m_window != nullptr; }

		/// <summary>
		/// Retrieves the current window extent (width and height).
		/// </summary>
		/// <returns>VkExtent2D structure with the window dimensions.</returns>
		VkExtent2D getExtent();

		/// <summary>
		/// Creates and retrieves the Vulkan surface associated with the window.
		/// </summary>
		/// <param name="instance">The Vulkan instance.</param>
		/// <returns>Handle to the created Vulkan surface.</returns>
		/// <exception cref="std::runtime_error">Thrown if surface creation fails.</exception>
		VkSurfaceKHR getWindowSurface(VkInstance instance);

		/// <summary>
		/// Retrieves the required instance extensions for Vulkan.
		/// </summary>
		/// <returns>A vector of required Vulkan instance extensions.</returns>
		/// <exception cref="std::runtime_error">Thrown if extension retrieval fails.</exception>
		std::vector<const char*> getRequiredInstanceExtensions();

		/// <summary>
		/// Retrieves the raw GLFW window handle.
		/// </summary>
		/// <returns>Pointer to the GLFW window.</returns>
		GLFWwindow* getGLFWWindow() { return m_window; }

		/// <summary>
		/// Sets the key callback function.
		/// </summary>
		/// <param name="keyCallback">GLFW key callback function.</param>
		void setKeyCallback(GLFWkeyfun keyCallback);

		/// <summary>
		/// Sets the cursor position callback function.
		/// </summary>
		/// <param name="cursorPosCallback">GLFW cursor position callback function.</param>
		void setCursorPositionCallback(GLFWcursorposfun cursorPosCallback);

		/// <summary>
		/// Sets the scroll callback function.
		/// </summary>
		/// <param name="scrollCallback">GLFW scroll callback function.</param>
		void setScrollCallback(GLFWscrollfun scrollCallback);

		/// <summary>
		/// Checks if the window was resized.
		/// </summary>
		/// <returns>True if the framebuffer was resized, otherwise false.</returns>
		bool wasResized() const { return m_framebufferResized; }

		/// <summary>
		/// Resets the window resize flag.
		/// </summary>
		void resetResizedFlag() { m_framebufferResized = false; }

		/// <summary>
		/// Polls for GLFW window events.
		/// </summary>
		void pollEvents() { glfwPollEvents(); }

		/// <summary>
		/// Checks if the window should close.
		/// </summary>
		/// <returns>True if the window should close, otherwise false.</returns>
		bool shouldClose() const { return glfwWindowShouldClose(m_window); }

		/// <summary>
		/// Retrieves the current time since GLFW initialization.
		/// </summary>
		/// <returns>Elapsed time in seconds.</returns>
		float currentTimeInSeconds();
	};
}
