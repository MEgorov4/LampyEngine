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

	bool glfwWindowIsValid() const { return m_window != nullptr; }

	VkExtent2D getExtent();
	VkSurfaceKHR getWindowSurface(VkInstance instance);
	std::vector<const char*> getRequiredInstanceExtensions();
	GLFWwindow* getGLFWWindow() { return m_window; }

	void setKeyCallback(GLFWkeyfun keyCallback);
	void setCursorPositionCallback(GLFWcursorposfun cursorPosCallback);
	void setScrollCallback(GLFWscrollfun scrollCallback);

	bool wasResized() const { return m_framebufferResized; }
	void resetResizedFlag() { m_framebufferResized = false; }
	void pollEvents(){glfwPollEvents();}
	bool shouldClose() const { return glfwWindowShouldClose(m_window);}

	float currentTimeInSeconds();
};

