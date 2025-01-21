#pragma once

#include "../ObjectCoreModule/ObjectModel/Scene.h"
#include "Vulkan/VulkanObjects/VulkanGraphicsPipeline.h"

class IRenderer
{
protected:
	Scene* m_rendererScene;
public:
	IRenderer() : m_rendererScene(nullptr) {};
	virtual ~IRenderer() {};

	virtual void render() = 0;

	virtual void registerShader(const std::string& vertPath, const std::string& fragPath) = 0;
	virtual void removeShader(const std::string& vertPath, const std::string& fragPath) = 0;

	virtual void registerVertexData(const std::vector<Vertex>& vertexData) = 0;
	virtual void removeVertexData(const std::vector<Vertex>& vertexData) = 0;

	virtual void setSceneToRender(Scene* scene) { m_rendererScene = scene; };
	

	virtual void waitIdle() = 0;
};