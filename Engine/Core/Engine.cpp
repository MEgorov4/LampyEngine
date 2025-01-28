#include "Engine.h"

#include "EngineConfig.h"
#include "../EngineContext/EngineContext.h"
#include "../Modules/WindowModule/WindowModule.h"
#include "../Modules/InputModule/InputModule.h"
#include "../Modules/RenderModule/RenderConfig.h"
#include "../Modules/RenderModule/RenderModule.h"
#include "../Modules/AudioModule/AudioModule.h"
#include "../Editor/Editor.h"

Engine::Engine(){}

Engine::~Engine(){}

void Engine::run()
{
	startupModules();
	startupEngineContextObject();
	engineTick();
	shutDownEngineContextObject();
	shutDownModules();
}

void Engine::startupModules()
{
	WindowModule::getInstance().startUp(800, 600, "Lampy Engine");

	InputModule::getInstance().startUp(WindowModule::getInstance().getWindow());

	RenderConfig& renderConfig = RenderConfig::getInstance();
	renderConfig.setMaxFramesInFlight(EngineConfig::getInstance().getMaxFramesInFlight());
	renderConfig.setGraphicsAPI(GraphicsAPI::Vulkan);
	RenderModule::getInstance().startUp(WindowModule::getInstance().getWindow());
	
	AudioModule::getInstance().startUp();
}

void Engine::startupEngineContextObject()
{
	m_engineContext = std::make_unique<Editor>();
	m_engineContext->init();
}
void Engine::engineTick()
{
	float deltaTime = 0.0f;
	float lastTime = WindowModule::getInstance().getWindow()->currentTimeInSeconds();

	while (!WindowModule::getInstance().getWindow()->shouldClose())
	{
		float currentTime = WindowModule::getInstance().getWindow()->currentTimeInSeconds();

		deltaTime = currentTime - lastTime;
		lastTime = currentTime;

		WindowModule::getInstance().getWindow()->pollEvents();
		m_engineContext->tick(deltaTime);
		RenderModule::getInstance().getRenderer()->render();
	}
	RenderModule::getInstance().getRenderer()->waitIdle();
}


void Engine::shutDownEngineContextObject()
{
	m_engineContext->shutDown();
}

void Engine::shutDownModules()
{
	AudioModule::getInstance().shutDown();
	RenderModule::getInstance().shutDown();
	InputModule::getInstance().shutDown();
	WindowModule::getInstance().shutDown();
}



