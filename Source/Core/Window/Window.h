#pragma once 

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vector>

class Window
{
	GLFWwindow* m_window;
	
	bool m_framebufferResized = false;
public:
	Window(int width, int height, const char* title);
	~Window();

	bool glfwWindowIsValid() { return m_window != nullptr; }

	void pollEvents(){glfwPollEvents();}
	bool shouldClose() const { return glfwWindowShouldClose(m_window);}

	std::vector<const char*> getRequiredInstanceExtensions();
	VkExtent2D getExtent();

	VkSurfaceKHR getWindowSurface(VkInstance instance);
	
	bool wasResized() const { return m_framebufferResized; }

	void resetResizedFlag() { m_framebufferResized = false; }

	GLFWwindow* getGLFWWindow() { return m_window; }


};

