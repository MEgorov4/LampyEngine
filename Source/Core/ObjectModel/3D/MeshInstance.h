#pragma once 
#include "Node3D.h"
#include "../Resources/Mesh.h"
#include "../Resources/Shader.h"

class MeshInstance : public Node3D
{
	std::unique_ptr<Mesh> m_mesh;
	std::shared_ptr<Shader> m_shader;
public:
	MeshInstance(Node* parentNode, std::unique_ptr<Mesh> mesh, std::shared_ptr<Shader> shader);

	virtual void render(VkCommandBuffer commandBuffer) override;
};