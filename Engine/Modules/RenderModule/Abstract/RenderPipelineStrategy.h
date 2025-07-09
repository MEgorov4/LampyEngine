#pragma once

#include <memory>
#include <flecs.h>
#include "../../ResourceModule/ResourceManager.h"
#include "../../ObjectCoreModule/ECS/ECSComponents.h"
#include "../../ObjectCoreModule/ECS/ECSModule.h"

#include "IFramebuffer.h"
#include "IMaterial.h"
#include "IShader.h"
#include "ITexture.h"
#include "IMesh.h"

#include "RenderObject.h"
#include "RenderPassStrategy.h"



namespace RenderModule
{
    class IRenderPipelineStrategy
    {
    protected:
        std::list<std::unique_ptr<IRenderPassStrategy>> m_renderPassStrategies;
        std::unordered_map<std::string, TextureHandle> m_textures;
    public:
        explicit IRenderPipelineStrategy(const std::shared_ptr<ResourceModule::ResourceManager>& resourceModule) 
        {
        }

        virtual ~IRenderPipelineStrategy() = default;

        TextureHandle execute(const ShaderUniformBlock_CameraData& cameraData) 
        {
            for (auto& renderPassStrategy : m_renderPassStrategies)
            {
                std::unordered_map<std::string, TextureHandle> outputTextures = renderPassStrategy->execute(cameraData, m_textures);
                for (auto& [key, value] : outputTextures)
                {
                    m_textures[key] = value;
                }
            }
            return m_textures["final_pass_color"];
        }
        
        void cleanup() const
        {
            for (auto& renderPassStrategy : m_renderPassStrategies)
            {
                renderPassStrategy->cleanup();
            }
        }

        void parseWorld(flecs::world& world) const 
        {
            for (auto& renderPassStrategy : m_renderPassStrategies)
            {
                renderPassStrategy->parseWorld(world);
            }
        }
    };

    class CleanRenderStrategy final : public IRenderPipelineStrategy
    {
    public:
        explicit CleanRenderStrategy(const std::shared_ptr<ResourceModule::ResourceManager>& resourceModule)
            : IRenderPipelineStrategy(resourceModule)
        {
            m_renderPassStrategies.push_back(std::make_unique<ShadowPassStrategy>(resourceModule));
            m_renderPassStrategies.push_back(std::make_unique<ReflectionPassStrategy>(resourceModule));
            m_renderPassStrategies.push_back(std::make_unique<LightPassStrategy>(resourceModule));
            m_renderPassStrategies.push_back(std::make_unique<TexturePassStrategy>(resourceModule));
            /*
            m_renderPassStrategies.push_back(std::make_unique<CustomPassStrategy>(resourceModule));
            */
            m_renderPassStrategies.push_back(std::make_unique<FinalPassStrategy>(resourceModule));
        }
        
        ~CleanRenderStrategy() override = default;
    };

    
    class EditorRenderStrategy final : public IRenderPipelineStrategy
    {
    public:
        explicit EditorRenderStrategy(const std::shared_ptr<ResourceModule::ResourceManager>& resourceModule)
            : IRenderPipelineStrategy(resourceModule)
        {
            /*
            m_renderPassStrategies.push_back(std::make_unique<ImGuiPassStrategy>(resourceModule));
            */
            m_renderPassStrategies.push_back(std::make_unique<ShadowPassStrategy>(resourceModule));
            m_renderPassStrategies.push_back(std::make_unique<ReflectionPassStrategy>(resourceModule));
            m_renderPassStrategies.push_back(std::make_unique<LightPassStrategy>(resourceModule));
            m_renderPassStrategies.push_back(std::make_unique<TexturePassStrategy>(resourceModule));
            m_renderPassStrategies.push_back(std::make_unique<CustomPassStrategy>(resourceModule));
            m_renderPassStrategies.push_back(std::make_unique<FinalPassStrategy>(resourceModule));
        }

        ~EditorRenderStrategy() override = default;
    };

    class RenderPipelineHandler
    {
        std::shared_ptr<Logger::Logger> m_logger;
        std::unique_ptr<IRenderPipelineStrategy> m_pipelineStrategy;
        ShaderUniformBlock_CameraData m_cameraData;

        void* m_resultDescriptor = nullptr;
    public:
        explicit RenderPipelineHandler(const std::shared_ptr<ResourceModule::ResourceManager>& resourceManager, const std::shared_ptr<Logger::Logger>& logger):
            m_cameraData(), m_logger(logger)
        {
            m_logger->log(Logger::LogVerbosity::Info, "Create strategy", "RenderModule_RenderPipelineHandler");
            m_pipelineStrategy = std::make_unique<CleanRenderStrategy>(resourceManager);
        }
         
        TextureHandle execute() const
        {
            return m_pipelineStrategy->execute(m_cameraData);
        }

        void cleanup() const
        {
            m_pipelineStrategy->cleanup();
        }

        void parseWorld(flecs::world& world)
        {
            using namespace ECSModule;

            auto query  = world.query_builder<PositionComponent, RotationComponent, CameraComponent>();
            
            query.each([this](const PositionComponent& pos, const RotationComponent& rot, const CameraComponent& cam)
            {
                glm::vec3 cameraPos{};
                glm::vec3 cameraTarget{};
                glm::vec3 upVector{};
                glm::mat4 view{};
                
                glm::mat4 proj = glm::perspective(glm::radians(cam.fov), cam.aspect, cam.nearClip, cam.farClip);

                cameraPos = pos.toGLMVec();
                cameraTarget = cameraPos + rot.toQuat() * glm::vec3(0.f, 0.f, 1.f);
                upVector = rot.toQuat() * glm::vec3(0.f, -1.f, 0.f);

                view = glm::lookAt(cameraPos, cameraTarget, upVector);

                m_cameraData.position = glm::vec4(cameraPos,1.0f);
                m_cameraData.projection = proj;
                m_cameraData.view = view;
            });

            m_pipelineStrategy->parseWorld(world);
        }
    };
}
