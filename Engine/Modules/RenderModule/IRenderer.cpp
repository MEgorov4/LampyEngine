#include "IRenderer.h"

#include "../ObjectCoreModule/ECS/ECSModule.h"
#include "../LoggerModule/Logger.h"

#include "Abstract/RenderResourcesFactory.h"

IRenderer::IRenderer()
{
	m_onECSChanged = ECSModule::getInstance().OnComponentsChanged.subscribe(std::bind_front(&IRenderer::updateRenderList, this));
}

IRenderer::~IRenderer()
{
	ECSModule::getInstance().OnComponentsChanged.unsubscribe(m_onECSChanged);
}

void IRenderer::updateRenderList()
{
	m_updateRenderObjects.clear();

	auto& world = ECSModule::getInstance().getCurrentWorld();

	glm::mat4 proj;

	glm::vec3 cameraPos{};
	glm::vec3 cameraTarget{};
	glm::vec3 upVector{};
	glm::mat4 view{};

	world.query<Position, Rotation, Camera>().each([&](Position& pos, Rotation& rot, Camera& cam)
		{
			proj = glm::perspective(glm::radians(cam.fov), cam.aspect, cam.nearClip, cam.farClip);

			cameraPos = pos.toGLMVec();
			cameraTarget = cameraPos + rot.toQuat() * glm::vec3(0, 0, 1);
			upVector = rot.toQuat() * glm::vec3(0, -1, 0);

			view = glm::lookAt(cameraPos, cameraTarget, upVector);
		});

	world.query<Position, Rotation, Scale, MeshComponent>().each([=](const flecs::entity& entity, Position& pos, Rotation& rot, Scale& scale, MeshComponent& mesh)
		{
			RenderObject renderObject;

			glm::mat4 model{ 1.f };

			model = glm::translate(model, pos.toGLMVec());

			model = glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

			glm::vec3 radiansRotation = glm::radians(rot.toEuler());

			model = glm::rotate(model, radiansRotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
			model = glm::rotate(model, radiansRotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::rotate(model, radiansRotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

			model = glm::scale(model, scale.toGLMVec());

			renderObject.modelMatrix = model;
			renderObject.projectionMatrix = proj;
			renderObject.viewMatrix = view;

			if (mesh.meshResource)
			{
				renderObject.mesh = MeshFactory::createOrGetMesh(mesh.meshResource.value());
			}

			if (mesh.vertShaderPath && mesh.fragShaderResource)
			{
				renderObject.shader = ShaderFactory::createOrGetShader(mesh.vertShaderResource.value(), mesh.fragShaderResource.value());
			}

			m_updateRenderObjects.push_back(renderObject);
		});

	std::swap(m_activeRenderObjects, m_updateRenderObjects);
}
