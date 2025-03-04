#pragma once
#include <memory>
#include <unordered_map>
#include <glm/glm.hpp>

#include "IMesh.h"
#include "IShader.h"

struct DirectionalLight
{
	glm::vec4 direction;
	glm::vec4 color;
	float intensity;
};

struct PointLight
{
	glm::vec4 position;
	glm::vec4 color;
	float intensity;
};

struct RenderObject
{
	glm::mat4 modelMatrix{ 1.0f };

	std::shared_ptr<IMesh> mesh;

	bool isVisible{ true };
	int renderLayer{ 0 };
};

enum RenderPassType
{
	SHADOW,
	REFLECTION,
	LIGHT,
	FINAL,
	CUSTOM
};
struct RenderPassData
{
	RenderPassType renderPassType{ CUSTOM };
	std::unordered_map<std::shared_ptr<IShader>, std::unordered_map<std::shared_ptr<IMesh>, std::vector<RenderObject>>> batches;

	void clear()
	{
		batches.clear();
	}
};


struct RenderPipelineData
{
	glm::mat4 projMatrix{ 1.0f };
	glm::mat4 viewMatrix{ 1.0f };
	glm::vec4 cameraPosition{ 1.0f };

	DirectionalLight directionalLight;
	std::vector<PointLight> pointLights;

	RenderPassData shadowPass{ SHADOW };
	RenderPassData reflectionPass{ REFLECTION };
	RenderPassData lightPass{ LIGHT };
	RenderPassData finalPass{ FINAL };
	RenderPassData customPass{ CUSTOM };

	void clear()
	{
		shadowPass.clear();
		reflectionPass.clear();
		lightPass.clear();
		finalPass.clear();
		customPass.clear();
	}
};