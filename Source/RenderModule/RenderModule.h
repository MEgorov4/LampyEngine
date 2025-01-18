#pragma once
#include <memory>
#include "RenderConfig.h"
#include "IRenderer.h"
#include "Vulkan/VulkanRenderer.h"

class RenderModule
{
	std::unique_ptr<IRenderer> m_renderer;

public:
	RenderModule(){}
	~RenderModule(){}
	static RenderModule& getInstance()
	{
		static RenderModule renderModule;
		return renderModule;
	}

	void startUp(Window* window)
	{
		RenderConfig& config = RenderConfig::getInstance();
		switch (config.getGraphicsAPI())
		{
			case(GraphicsAPI::Vulkan):
				m_renderer = std::make_unique<VulkanRenderer>(window);
			//case(GraphicsAPI::OpenGL):
			//	m_renderer = std::make_unique<IRenderer>();
		}
	}

	IRenderer* getRenderer() { return m_renderer.get(); }

	void shutDown()
	{
		m_renderer.reset();
	}
};