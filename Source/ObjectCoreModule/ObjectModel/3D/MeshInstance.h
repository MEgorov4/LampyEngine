#pragma once 
#include "Node3D.h"

#include <memory>

class Mesh;
class Shader;

class MeshInstance : public Node3D
{
	std::unique_ptr<Mesh> m_mesh;
	std::unique_ptr<Shader> m_shader;
public:
	MeshInstance(Node* parentNode, std::unique_ptr<Mesh> mesh, std::unique_ptr<Shader> shader);
};