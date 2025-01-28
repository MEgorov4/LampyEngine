#pragma once
#include <memory>
#include "../EngineContext/EngineContext.h"
class Engine
{
	std::unique_ptr<IEngineContext> m_engineContext;
public:
	Engine();
	Engine(const Engine& app) = delete;
	~Engine();
	const Engine& operator=(const Engine rhs) = delete;

	void run();

private:
	void startupModules();
	void startupEngineContextObject();
	void engineTick();
	void shutDownEngineContextObject();
	void shutDownModules();
};