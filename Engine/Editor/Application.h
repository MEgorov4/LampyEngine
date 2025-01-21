#pragma once

class Scene;

class Application
{
	Scene* m_scene;
public:
	Application();
	Application(const Application& app) = delete;
	~Application();
	const Application& operator=(const Application rhs) = delete;

	void run();

private:
	void startupModules();
	void startupEngine();
	void applicationTick();
	void shutDownModules();
};