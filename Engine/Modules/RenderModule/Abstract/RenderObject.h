#pragma once
#include <memory>
#include <glm/glm.hpp>

#include "IMesh.h"
#include "IShader.h"

struct RenderObject
{
	glm::mat4 modelMatrix{ 1.0f };
	glm::mat4 viewMatrix{ 1.0f };
	glm::mat4 projectionMatrix{ 1.0f };
	
	std::shared_ptr<IMesh> mesh;
	
	std::shared_ptr<IShader> shader;

	bool isVisible{ true };
	int renderLayer { 0 };
};