#include "IRenderer.h"

#include "../ObjectCoreModule/ECS/ECSModule.h"
#include "../LoggerModule/Logger.h"

#include "Abstract/RenderResourcesFactory.h"
#include "../ResourceModule/ResourceManager.h"

IRenderer::IRenderer()
{
}

IRenderer::~IRenderer()
{
	ECSModule::getInstance().OnComponentsChanged.unsubscribe(m_onECSChanged);
}

void IRenderer::updateRenderList()
{
	m_updateRenderPipelineData.clear();

	auto& world = ECSModule::getInstance().getCurrentWorld();

	glm::mat4 proj;

	glm::vec3 cameraPos{};
	glm::vec3 cameraTarget{};
	glm::vec3 upVector{};
	glm::mat4 view{};

	world.query<PositionComponent, RotationComponent, CameraComponent>().each([&](PositionComponent& pos, RotationComponent& rot, CameraComponent& cam)
		{
			proj = glm::perspective(glm::radians(cam.fov), cam.aspect, cam.nearClip, cam.farClip);

			cameraPos = pos.toGLMVec();
			cameraTarget = cameraPos + rot.toQuat() * glm::vec3(0.f, 0.f, 1.f);
			upVector = rot.toQuat() * glm::vec3(0.f, -1.f, 0.f);

			view = glm::lookAt(cameraPos, cameraTarget, upVector);
		});

	m_updateRenderPipelineData.projMatrix = proj;
	m_updateRenderPipelineData.viewMatrix = view;
	m_updateRenderPipelineData.cameraPosition = glm::vec4(cameraPos, 1.0f);


	DirectionalLight dirLight;

	world.query<PositionComponent, RotationComponent, DirectionalLightComponent>().each([&](PositionComponent& pos, RotationComponent& rot, DirectionalLightComponent& lightComponent)
		{
			glm::quat quat = rot.toQuat();
			dirLight.direction = glm::vec4(quat * glm::vec3(0.0f, 0.0f, -1.0f), 1.0f);
			dirLight.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
			dirLight.intensity = lightComponent.intencity;
		});
	m_updateRenderPipelineData.directionalLight;
	world.query<PositionComponent, RotationComponent, ScaleComponent, MeshComponent>().each([=](const flecs::entity& entity, PositionComponent& pos, RotationComponent& rot, ScaleComponent& scale, MeshComponent& mesh)
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

			std::shared_ptr<IShader> customShader;
			std::shared_ptr<IMesh> customMesh;

			if (mesh.vertShaderPath && mesh.fragShaderResource)
			{
				customShader = ShaderFactory::createOrGetShader(mesh.vertShaderResource.value(), mesh.fragShaderResource.value());
				if (mesh.meshResource)
				{
					customMesh = MeshFactory::createOrGetMesh(mesh.meshResource.value());
				}
			}

			m_updateRenderPipelineData
				.customPass
				.batches[customShader][customMesh]
				.push_back(renderObject);

			m_updateRenderPipelineData
				.shadowPass
				.batches[m_shadowsShader][customMesh]
				.push_back(renderObject);

			m_updateRenderPipelineData
				.reflectionPass
				.batches[m_reflectionsShader][customMesh]
				.push_back(renderObject);

			m_updateRenderPipelineData
				.lightPass
				.batches[m_lightsShader][customMesh]
				.push_back(renderObject);

			m_updateRenderPipelineData
				.finalPass
				.batches[m_finalShader][customMesh]
				.push_back(renderObject);
		});

	std::swap(m_activeRenderPipelineData, m_updateRenderPipelineData);
}

void IRenderer::postInit()
{
	std::shared_ptr<RShader> SSHRV = ResourceManager::load<RShader>("../Resources/Shaders/GLSL/Core/shadow.vert");
	std::shared_ptr<RShader> SSHRF = ResourceManager::load<RShader>("../Resources/Shaders/GLSL/Core/shadow.frag");
	m_shadowsShader = ShaderFactory::createOrGetShader(SSHRV, SSHRF);

	std::shared_ptr<RShader> RSHRV = ResourceManager::load<RShader>("../Resources/Shaders/GLSL/Core/reflection.vert");
	std::shared_ptr<RShader> RSHRF = ResourceManager::load<RShader>("../Resources/Shaders/GLSL/Core/reflection.frag");
	m_reflectionsShader = ShaderFactory::createOrGetShader(RSHRV, RSHRF);

	std::shared_ptr<RShader> LSHRV = ResourceManager::load<RShader>("../Resources/Shaders/GLSL/Core/light.vert");
	std::shared_ptr<RShader> LSHRF = ResourceManager::load<RShader>("../Resources/Shaders/GLSL/Core/light.frag");
	m_lightsShader = ShaderFactory::createOrGetShader(LSHRV, LSHRF);

	std::shared_ptr<RShader> FSHRV = ResourceManager::load<RShader>("../Resources/Shaders/GLSL/Core/final.vert");
	std::shared_ptr<RShader> FSHRF = ResourceManager::load<RShader>("../Resources/Shaders/GLSL/Core/final.frag");
	m_finalShader = ShaderFactory::createOrGetShader(FSHRV, FSHRF);

	std::shared_ptr<RTexture> DAT = ResourceManager::load<RTexture>("../Resources/Textures/Generic/DefaultAlbedo.png");
	m_albedoGeneric = TextureFactory::createOrGetTexture(DAT);

	std::shared_ptr<RTexture> DET = ResourceManager::load<RTexture>("../Resources/Textures/Generic/DefaultAlbedo.png");
	m_emissionGeneric = TextureFactory::createOrGetTexture(DET);

	m_onECSChanged = ECSModule::getInstance().OnComponentsChanged.subscribe(std::bind_front(&IRenderer::updateRenderList, this));


	std::shared_ptr<RShader> debugLineShaderVert = ResourceManager::load<RShader>("../Resources/Shaders/GLSL/Core/debugLine.vert");
	std::shared_ptr<RShader> debugLineShaderFrag = ResourceManager::load<RShader>("../Resources/Shaders/GLSL/Core/debugLine.frag");
	m_debugLineShader = ShaderFactory::createOrGetShader(debugLineShaderVert, debugLineShaderFrag);
}
