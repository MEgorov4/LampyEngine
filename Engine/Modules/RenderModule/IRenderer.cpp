#include "IRenderer.h"

#include "../ObjectCoreModule/ECS/ECSModule.h"
#include "../ObjectCoreModule/ECS/ECSComponents.h"
#include "../LoggerModule/Logger.h"

#include "Abstract/RenderResourcesFactory.h"
#include "../ResourceModule/ResourceManager.h"

namespace RenderModule
{
	IRenderer::IRenderer(std::shared_ptr<ResourceModule::ResourceManager> resourceManager, std::shared_ptr<ECSModule::ECSModule> ecsModule) : m_resourceManager(resourceManager), m_ecsModule(ecsModule)
	{
		
	}

	IRenderer::~IRenderer()
	{
		m_ecsModule->OnComponentsChanged.unsubscribe(m_onECSChanged);
	}

	void IRenderer::updateRenderList()
	{
		m_updateRenderPipelineData.clear();

		auto& world = m_ecsModule->getCurrentWorld();

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

		world.query<RotationComponent, DirectionalLightComponent>().each([&](RotationComponent& rot, DirectionalLightComponent& lightComponent)
			{
				glm::quat quat = rot.toQuat();
				dirLight.direction = glm::vec4(quat * glm::vec3(0.0f, 0.0f, -1.0f), 1.0f);
				dirLight.color = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
				dirLight.intensity = lightComponent.intencity;
			});

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
				std::shared_ptr<ITexture> texture;
				if (mesh.meshResource)
				{
					customMesh = MeshFactory::createOrGetMesh(mesh.meshResource.value());

					if (mesh.textureResource)
					{
						renderObject.texture = TextureFactory::createOrGetTexture(mesh.textureResource.value());
					}
					
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
						.texturePass
						.batches[m_textureShader][customMesh]
						.push_back(renderObject);

					m_updateRenderPipelineData
						.finalPass
						.batches[m_finalShader][customMesh]
						.push_back(renderObject);

					if (mesh.vertShaderPath && mesh.fragShaderResource)
					{
						customShader = ShaderFactory::createOrGetShader(mesh.vertShaderResource.value(), mesh.fragShaderResource.value());
						m_updateRenderPipelineData
							.customPass
							.batches[customShader][customMesh]
							.push_back(renderObject);
					}
				}

			});

		std::swap(m_activeRenderPipelineData, m_updateRenderPipelineData);
	}

	void IRenderer::postInit()
	{
		using namespace ResourceModule;
		std::shared_ptr<RShader> SSHRV = m_resourceManager->load<RShader>("../Resources/Shaders/GLSL/Core/shadow.vert");
		std::shared_ptr<RShader> SSHRF =  m_resourceManager->load<RShader>("../Resources/Shaders/GLSL/Core/shadow.frag");
		m_shadowsShader = ShaderFactory::createOrGetShader(SSHRV, SSHRF);
		
		std::shared_ptr<RShader> RSHRV = m_resourceManager->load<RShader>("../Resources/Shaders/GLSL/Core/reflection.vert");
		std::shared_ptr<RShader> RSHRF = m_resourceManager->load<RShader>("../Resources/Shaders/GLSL/Core/reflection.frag");
		m_reflectionsShader = ShaderFactory::createOrGetShader(RSHRV, RSHRF);

		std::shared_ptr<RShader> LSHRV = m_resourceManager->load<RShader>("../Resources/Shaders/GLSL/Core/light.vert");
		std::shared_ptr<RShader> LSHRF = m_resourceManager->load<RShader>("../Resources/Shaders/GLSL/Core/light.frag");
		m_lightsShader = ShaderFactory::createOrGetShader(LSHRV, LSHRF);

		std::shared_ptr<RShader> TSHRV = m_resourceManager->load<RShader>("../Resources/Shaders/GLSL/Core/texture.vert");
		std::shared_ptr<RShader> TSHRF = m_resourceManager->load<RShader>("../Resources/Shaders/GLSL/Core/texture.frag");
		m_textureShader = ShaderFactory::createOrGetShader(TSHRV, TSHRF);

		std::shared_ptr<RShader> FSHRV = m_resourceManager->load<RShader>("../Resources/Shaders/GLSL/Core/final.vert");
		std::shared_ptr<RShader> FSHRF = m_resourceManager->load<RShader>("../Resources/Shaders/GLSL/Core/final.frag");
		m_finalShader = ShaderFactory::createOrGetShader(FSHRV, FSHRF);

		std::shared_ptr<RTexture> DAT = m_resourceManager->load<RTexture>("../Resources/Textures/Generic/DefaultAlbedo.png");
		m_albedoGeneric = TextureFactory::createOrGetTexture(DAT);

		std::shared_ptr<RTexture> DET = m_resourceManager->load<RTexture>("../Resources/Textures/Generic/DefaultAlbedo.png");
		m_emissionGeneric = TextureFactory::createOrGetTexture(DET);

		std::shared_ptr<RTexture> DGBT = m_resourceManager->load<RTexture>("../Resources/Textures/Generic/GrayBoxTexture.png");
		m_emptyTextureGeneric = TextureFactory::createOrGetTexture(DGBT);

		std::shared_ptr<RShader> debugLineShaderVert = m_resourceManager->load<RShader>("../Resources/Shaders/GLSL/Core/debugLine.vert");
		std::shared_ptr<RShader> debugLineShaderFrag = m_resourceManager->load<RShader>("../Resources/Shaders/GLSL/Core/debugLine.frag");
		m_debugLineShader = ShaderFactory::createOrGetShader(debugLineShaderVert, debugLineShaderFrag);
		
		m_onECSChanged = m_ecsModule->OnComponentsChanged.subscribe(std::bind_front(&IRenderer::updateRenderList, this));


	}
}
