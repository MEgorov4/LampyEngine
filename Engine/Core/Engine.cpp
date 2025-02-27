#include "Engine.h"

#include "EngineConfig.h"
#include "../Editor/Editor.h"
#include "../EngineContext/EngineContext.h"
#include "../Modules/WindowModule/WindowModule.h"
#include "../Modules/InputModule/InputModule.h"
#include "../Modules/RenderModule/RenderConfig.h"
#include "../Modules/RenderModule/RenderModule.h"
#include "../Modules/AudioModule/AudioModule.h"
#include "../Modules/ObjectCoreModule/ECS/ECSModule.h"
#include "../Modules/LoggerModule/Logger.h"
#include "../Modules/LuaScriptModule/LuaScriptModule.h"
#include "../Modules/ResourceModule/ResourceManager.h"
// #include "../Modules/ResourceModule/Shader.h"

Engine::Engine(){}

Engine::~Engine(){}

void Engine::run()
{
	LOG_INFO("Engine: Startup engine context");
	startupEngineContextObject();
	LOG_INFO("Engine: Startup modules");
	startupModules();
	initMajorEngineContext();
	LOG_INFO("Engine: Startup engine tick");
	engineTick();
	LOG_INFO("Engine: Shut down engine context");
	shutDownEngineContextObject();
	LOG_INFO("Engine: Shut down modules");
	shutDownModules();
}

void Engine::startupModules()
{
	WindowModule::getInstance().startup(800, 600, "Lampy Engine", GraphicsAPI::OpenGL);

	InputModule::getInstance().startup(WindowModule::getInstance().getWindow());

	ResourceManager::getInstance().startup();

	RenderConfig& renderConfig = RenderConfig::getInstance();
	renderConfig.setMaxFramesInFlight(EngineConfig::getInstance().getMaxFramesInFlight());
	renderConfig.setGraphicsAPI(GraphicsAPI::OpenGL);
	RenderModule::getInstance().startup(WindowModule::getInstance().getWindow());
	
	AudioModule::getInstance().startup();
	ECSModule::getInstance().startup(); 
	idOnLoadInitialWorldState = ECSModule::getInstance().OnLoadInitialWorldState.subscribe(std::bind(&ResourceManager::OnLoadInitialWorldState, ResourceManager::getInstance()));
	LuaScriptModule::getInstance().startup();

	ECSModule::getInstance().OnLoadInitialWorldState();
}

void Engine::startupEngineContextObject()
{
	m_engineContext = std::make_unique<Editor>();
	m_engineContext->initMinor();
}
void Engine::initMajorEngineContext()
{
	m_engineContext->initMajor();
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
		ECSModule::getInstance().ecsTick(deltaTime);
		RenderModule::getInstance().getRenderer()->updateRenderList();
	}
	RenderModule::getInstance().getRenderer()->waitIdle();
}


void Engine::shutDownEngineContextObject()
{
	m_engineContext->shutDown();
}

void Engine::shutDownModules()
{
	ECSModule::getInstance().OnLoadInitialWorldState.unsubscribe(idOnLoadInitialWorldState);
	ECSModule::getInstance().shutDown();
	AudioModule::getInstance().shutDown();
	RenderModule::getInstance().shutDown();
	ResourceManager::getInstance().shutDown();
	InputModule::getInstance().shutDown();
	WindowModule::getInstance().shutDown();
}



