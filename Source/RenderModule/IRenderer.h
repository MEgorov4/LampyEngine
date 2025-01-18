#pragma once

#include "../ObjectCoreModule/ObjectModel/Scene.h"

class IRenderer
{
protected:
	Scene* m_rendererScene;
public:
	IRenderer() : m_rendererScene(nullptr) {};
	virtual ~IRenderer() {};

	virtual void render() = 0;
	virtual void setSceneToRender(Scene* scene) { m_rendererScene = scene; };

	virtual void waitIdle() = 0;
};