#include "Window.h"
#include <stdexcept>
#include <iostream>
#include "format"

#include "../LoggerModule/Logger.h"

Window::Window(int width, int height, const char* title, GraphicsAPI api)
{	
	
	LOG_INFO("Window: Start create window: width = " + std::format("{}", width) + ", height = " + std::format("{}", height) + ", title = " + std::format("{}", title));

	if (!glfwInit())
	{
		throw std::runtime_error("failed to initialise glfw");
	}

	if (api == GraphicsAPI::Vulkan)
	{
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	}

	m_window = glfwCreateWindow(width, height, title, nullptr, nullptr);

	if (api == GraphicsAPI::OpenGL)
	{
		glfwMakeContextCurrent(m_window);
	}

	LOG_INFO("Window: Window created");

	glfwSetWindowUserPointer(m_window, this);

	//////////////////////////////////////////////// SHIT EBANII
	auto framebufferResizeCallback = [](GLFWwindow* window, int width, int height)
	{
			auto appWindow = reinterpret_cast<Window*>(glfwGetWindowUserPointer(window));
			appWindow->m_framebufferResized = true;
	};

	glfwSetFramebufferSizeCallback(m_window, framebufferResizeCallback);
}

Window::~Window()
{
	LOG_INFO("Window: Start destroy window");

	glfwDestroyWindow(m_window);
	glfwTerminate();

	LOG_INFO("Window: Window desrtoyed");
}


std::vector<const char*> Window::getRequiredInstanceExtensions()
{
	uint32_t extensionsCount;

	LOG_INFO("Window: Try get extensions");

	const char** extensionsNames = glfwGetRequiredInstanceExtensions(&extensionsCount);

	for (size_t i = 0; i < extensionsCount; i++)
	{
		LOG_INFO("Window: Extension " + std::format("{} {}", i + 1, extensionsNames[i]));
	}

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

	return VkExtent2D{static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}

VkSurfaceKHR Window::getWindowSurface(VkInstance instance)
{
	VkSurfaceKHR surface;
	
	LOG_INFO("Window: Try to create window surface");

	if (glfwCreateWindowSurface(instance, m_window, nullptr, &surface) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create surface");
	}

	LOG_INFO("Window: Window surface created");

	return surface;
}

