#include "Window.h"
#include <stdexcept>
#include <iostream>
#include <format>

#include "../LoggerModule/Logger.h"
#include "../RenderModule/RenderConfig.h"

namespace WindowModule
{
	Window::Window(std::shared_ptr<Logger::Logger> logger, int width, int height, const char* title) : m_logger(logger)
	{

		m_logger->log(Logger::LogVerbosity::Info
			, "Window: Start create window: width = "
			+ std::format("{}", width)
			+ ", height = "
			+ std::format("{}", height)
			+ ", title = "
			+ std::format("{}", title)
			, "WindowModule_Window");

		if (!glfwInit())
		{
			throw std::runtime_error("failed to initialise glfw");
		}


		m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);

		glfwMakeContextCurrent(m_window);

		m_logger->log(Logger::LogVerbosity::Info, "Window created", "WindowModule_Window");

		glfwSetWindowUserPointer(m_window, this);

		m_logger->log(Logger::LogVerbosity::Info, "Set resize callback", "WindowModule_Window");
		//////////////////////////////////////////////// SHIT EBANII
		auto framebufferResizeCallback = [](GLFWwindow* window, int width, int height)
			{
				auto appWindow = static_cast<Window*>(glfwGetWindowUserPointer(window));
				appWindow->m_framebufferResized = true;
			};

		glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
	}

	Window::~Window()
	{
		
		glfwDestroyWindow(m_window);
		glfwTerminate();

		m_logger->log(Logger::LogVerbosity::Info, "Window destroyed", "WindowModule_Window");
	}


	std::vector<const char*> Window::getRequiredInstanceExtensions()
	{
		uint32_t extensionsCount;

		m_logger->log(Logger::LogVerbosity::Info, "Try get extesions", "WindowModule_Window");

		const char** extensionsNames = glfwGetRequiredInstanceExtensions(&extensionsCount);

		if (!extensionsNames || extensionsCount == 0) {
			throw std::runtime_error("Failed to get required instance extensions from GLFW");
		}

		return std::vector<const char*>(extensionsNames, extensionsNames + extensionsCount);
	}

	void Window::setKeyCallback(GLFWkeyfun keyCallback)
	{
		glfwSetKeyCallback(m_window, keyCallback);
	}

	void Window::setCursorPositionCallback(GLFWcursorposfun cursorPosCallback)
	{
		glfwSetCursorPosCallback(m_window, cursorPosCallback);
	}

	void Window::setScrollCallback(GLFWscrollfun scrollCallback)
	{
		glfwSetScrollCallback(m_window, scrollCallback);
	}

	float Window::currentTimeInSeconds()
	{
		return glfwGetTime();
	}

	VkExtent2D Window::getExtent()
	{
		int width, height;
		glfwGetWindowSize(m_window, &width, &height);

		return VkExtent2D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height) };
	}

	VkSurfaceKHR Window::getWindowSurface(VkInstance instance)
	{
		VkSurfaceKHR surface;

		if (glfwCreateWindowSurface(instance, m_window, nullptr, &surface) != VK_SUCCESS)
		{
			throw std::runtime_error("failed to create surface");
		}

		m_logger->log(Logger::LogVerbosity::Info, "Window surface created", "WindowModule_Window");

		return surface;
	}
}
