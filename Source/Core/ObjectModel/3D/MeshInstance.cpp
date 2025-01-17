#include "MeshInstance.h"

MeshInstance::MeshInstance(Node* parentNode, std::unique_ptr<Mesh> mesh, std::shared_ptr<Shader> shader) : Node3D(parentNode), m_mesh(std::move(mesh)), m_shader(shader){}

void MeshInstance::render(VkCommandBuffer commandBuffer)
{
	m_shader->bindShader(commandBuffer);
	m_mesh->renderMesh(commandBuffer, getGlobalTransform());

	Node::render(commandBuffer);
}
