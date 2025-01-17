#pragma once
#include <vulkan/vulkan.hpp>

#include "3D/Node3D.h"
#include "3D/Camera3D.h"

class Scene
{
	std::vector<Node3D*> m_nodes;
	Camera3D* m_cameraNode;

public:
	Scene();
	~Scene();

	void RenderScene(VkCommandBuffer commandBuffer);
};