#pragma once

#include "../../Core/ObjectModel/Scene.h"

class IRenderer
{
public:
	IRenderer() {};
	virtual ~IRenderer() {};

	virtual void renderScene(const Scene* scene) = 0;
};