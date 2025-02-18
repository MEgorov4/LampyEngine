#include "RenderModule.h"

#include "Vulkan/VulkanRenderer.h"

void RenderModule::startup(Window* window)
{
	const RenderConfig& config = RenderConfig::getInstance();
	LOG_INFO(std::format("RenderModule: Startup with - {}", config.getGraphicsAPI() == GraphicsAPI::Vulkan ? "Vulkan" : "OpenGL"));

	switch (config.getGraphicsAPI())
	{
	case GraphicsAPI::Vulkan:
		m_renderer = std::make_unique<VulkanRenderer>(window);
		break;

		// case GraphicsAPI::OpenGL:
		//     m_renderer = std::make_unique<OpenGLRenderer>(); (Implementation required)
	}
}
