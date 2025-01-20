#include "Application.h"

#include "../ObjectCoreModule/ObjectModel/Scene.h"
#include "../RenderModule/Vulkan/VulkanRenderer.h"
#include "../RenderModule/RenderConfig.h"
#include "../RenderModule/RenderModule.h"
#include "../AudioModule/AudioModule.h"
#include "../WindowModule/WindowModule.h"
#include "../InputModule/InputModule.h"

#include "VulkanApplicationConfig.h"
Application::Application() : m_scene(nullptr){}

Application::~Application(){}

void Application::run()
{
	startupModules();
	startupEngine();
	applicationTick();
	shutDownModules();
}

void Application::startupModules()
{
	WindowModule::getInstance().startUp(800, 600, "Lampy Engine");

	InputModule::getInstance().startUp(WindowModule::getInstance().getWindow());

	RenderConfig& renderConfig = RenderConfig::getInstance();
	renderConfig.setMaxFramesInFlight(EngineApplicationConfig::getInstance().getMaxFramesInFlight());
	renderConfig.setGraphicsAPI(GraphicsAPI::Vulkan);
	RenderModule::getInstance().startUp(WindowModule::getInstance().getWindow());
	m_scene = new Scene();
	RenderModule::getInstance().getRenderer()->setSceneToRender(m_scene);
	
	AudioModule::getInstance().startUp();
}

void Application::startupEngine()
{
	InputModule::getInstance().setKeyCallback([](int key, int scancode, int action,int mods) {
		if (action == GLFW_PRESS)
		{
			if (key == GLFW_KEY_SPACE)
			{
				AudioModule::getInstance().playSoundAsync();
			}
		}
		});
}
void Application::applicationTick()
{
	while (!WindowModule::getInstance().getWindow()->shouldClose())
	{
		WindowModule::getInstance().getWindow()->pollEvents();

		RenderModule::getInstance().getRenderer()->render();
	}
	RenderModule::getInstance().getRenderer()->waitIdle();
}

void Application::shutDownModules()
{
	AudioModule::getInstance().shutDown();
	RenderModule::getInstance().shutDown();
	InputModule::getInstance().shutDown();
	WindowModule::getInstance().shutDown();
}



