#include "Scene.h"
#include "Resources/ResourceManager.h"

Scene::Scene()
{	
	for (uint32_t i = 1; i <= 1; i++)
	{
		const std::vector<Vertex> vertices{
			{{0.1f, -0.8f}, { 1.0f, 0.0f, 0.0f }},
			{ {0.8f, 0.2f}, {0.0f, 1.0f, 0.0f} },
			{ {-0.1f, 0.9f}, {0.0f, 0.0f, 1.0f} } 
		};

		m_nodes.push_back(new MeshInstance(nullptr, RM_CREATE_MESH(vertices), RM_CREATE_SHADER("vert.spv", "frag.spv")));
	}
	m_cameraNode = new Camera3D();
}
Scene::~Scene()
{
	for (Node3D* node : m_nodes)
	{
		delete node;
	}
	delete m_cameraNode;
}

void Scene::RenderScene(VkCommandBuffer commandBuffer)
{
	for (auto node : m_nodes)
	{
		node->render(commandBuffer);
	}
}

