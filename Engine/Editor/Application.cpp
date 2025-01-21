#include "Application.h"

#include "VulkanApplicationConfig.h"
#include "../Modules/WindowModule/WindowModule.h"
#include "../Modules/InputModule/InputModule.h"
#include "../Modules/RenderModule/RenderConfig.h"
#include "../Modules/RenderModule/RenderModule.h"
#include "../Modules/AudioModule/AudioModule.h"

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



