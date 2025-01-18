#pragma once

#include "memory"

class IRenderer;
class Scene;
class Window;
class RenderModule;

class Application
{
	std::unique_ptr<Window> m_window;

	Scene* m_scene;
public:
	Application();
	Application(const Application& app) = delete;
	~Application();
	const Application& operator=(const Application rhs) = delete;

	void run();

private:
	void initWindow();
	void initModules();
	void mainLoop();
	void shutDownModules();
};