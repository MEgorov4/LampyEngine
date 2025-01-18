#include "MeshInstance.h"

#include "../../../ResourceModule/Mesh.h"
#include "../../../ResourceModule/Shader.h"

MeshInstance::MeshInstance(Node* parentNode, std::unique_ptr<Mesh> mesh, std::unique_ptr<Shader> shader) : Node3D(parentNode), 
																										   m_mesh(std::move(mesh)), 
																										   m_shader(std::move(shader)){}

