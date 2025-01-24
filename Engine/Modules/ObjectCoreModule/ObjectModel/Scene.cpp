#include "Scene.h"

Scene::Scene()
{	
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


