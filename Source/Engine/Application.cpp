#include "Application.h"

#include "../ObjectCoreModule/ObjectModel/Scene.h"
#include "../WindowModule/Window.h"
#include "../RenderModule/Vulkan/VulkanRenderer.h"
#include "../RenderModule/RenderConfig.h"
#include "../RenderModule/RenderModule.h"
#include "VulkanApplicationConfig.h"

Application::Application() : m_scene(nullptr){}

Application::~Application(){}

void Application::run()
{
	initWindow();
	initModules();
	mainLoop();
}


void Application::initWindow()
{
	m_window = std::make_unique<Window>(800, 600, "Lampy Engine");
}

void Application::initModules()
{
	//Setup render module
	RenderConfig& renderConfig = RenderConfig::getInstance();
	renderConfig.setMaxFramesInFlight(EngineApplicationConfig::getInstance().getMaxFramesInFlight());
	renderConfig.setGraphicsAPI(GraphicsAPI::Vulkan);
	RenderModule::getInstance().startUp(m_window.get());
	m_scene = new Scene();
	RenderModule::getInstance().getRenderer()->setSceneToRender(m_scene);
}

void Application::mainLoop()
{
	while (!m_window->shouldClose())
	{
		m_window->pollEvents();

		RenderModule::getInstance().getRenderer()->render();
	}
	RenderModule::getInstance().getRenderer()->waitIdle();
}

void Application::shutDownModules()
{
	RenderModule::getInstance().shutDown();
}



