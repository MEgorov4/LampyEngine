#pragma once 
#include "Node3D.h"

#include <memory>

class RMesh;
class RShader;

class MeshInstance : public Node3D
{
	std::unique_ptr<RMesh> m_mesh;
	std::unique_ptr<RShader> m_shader;
public:
	explicit MeshInstance(Node* parentNode, std::unique_ptr<RMesh> mesh, std::unique_ptr<RShader> shader);
};