#pragma once

#include <memory>
#include <flecs.h>
#include "../../ResourceModule/ResourceManager.h"
#include "../../ObjectCoreModule/ECS/Components/ECSComponents.h"
#include "../../ObjectCoreModule/ECS/ECSModule.h"

#include "IFramebuffer.h"
#include "IMaterial.h"
#include "IShader.h"
#include "ITexture.h"
#include "IMesh.h"

#include "RenderObject.h"
#include "RenderPassStrategy.h"
#include "../../../EngineContext/CoreGlobal.h"



namespace RenderModule
{
    class IRenderPipelineStrategy
    {
    protected:
        std::list<std::unique_ptr<IRenderPassStrategy>> m_renderPassStrategies;
        std::unordered_map<std::string, TextureHandle> m_textures;
    public:
        explicit IRenderPipelineStrategy() 
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
        
        void resize(const std::pair<int, int>& renderSize)
        {
            for (auto& renderPassStrategy : m_renderPassStrategies)
            {
                renderPassStrategy->resize(renderSize);
            }
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
        explicit CleanRenderStrategy()
            : IRenderPipelineStrategy()
        {
            m_renderPassStrategies.push_back(std::make_unique<ShadowPassStrategy>());
            m_renderPassStrategies.push_back(std::make_unique<ReflectionPassStrategy>());
            m_renderPassStrategies.push_back(std::make_unique<LightPassStrategy>());
            m_renderPassStrategies.push_back(std::make_unique<TexturePassStrategy>());
            m_renderPassStrategies.push_back(std::make_unique<FinalPassStrategy>());
        }
        
        ~CleanRenderStrategy() override = default;
    };

    
    class EditorRenderStrategy final : public IRenderPipelineStrategy
    {
    public:
        explicit EditorRenderStrategy()
            : IRenderPipelineStrategy()
        {
            m_renderPassStrategies.push_back(std::make_unique<ShadowPassStrategy>());
            m_renderPassStrategies.push_back(std::make_unique<ReflectionPassStrategy>());
            m_renderPassStrategies.push_back(std::make_unique<LightPassStrategy>());
            m_renderPassStrategies.push_back(std::make_unique<TexturePassStrategy>());
            m_renderPassStrategies.push_back(std::make_unique<CustomPassStrategy>());
            m_renderPassStrategies.push_back(std::make_unique<FinalPassStrategy>());
        }

        ~EditorRenderStrategy() override = default;
    };

    class RenderPipelineHandler
    {
        std::unique_ptr<IRenderPipelineStrategy> m_pipelineStrategy;
        ShaderUniformBlock_CameraData m_cameraData;
        std::pair<int, int> m_renderSize;

        void* m_resultDescriptor = nullptr;
    public:
        RenderPipelineHandler()
        {
            GCM(Logger::Logger)->log(Logger::LogVerbosity::Info, "Create strategy", "RenderModule_RenderPipelineHandler");
            m_pipelineStrategy = std::make_unique<CleanRenderStrategy>();
        }
         
        TextureHandle execute() const
        {
            return m_pipelineStrategy->execute(m_cameraData);
        }

        void resize(int w, int h)
        {
            if (m_renderSize == std::pair<int, int>(w, h))
                return;

            m_renderSize = { w, h };
            m_pipelineStrategy->resize(m_renderSize);
            
            parseCameraData();
        }

        void cleanup() const
        {
            m_pipelineStrategy->cleanup();
        }

        void parseWorld(flecs::world& world)
        {

            m_pipelineStrategy->parseWorld(world);
            parseCameraData();
        }

    private:
        void parseCameraData()
        {
            auto query  = GCM(ECSModule::ECSModule)->getCurrentWorld().query_builder<PositionComponent, RotationComponent, CameraComponent>();

            using namespace ECSModule;
            
   query.each([this](const PositionComponent& pos, const RotationComponent& rot, const CameraComponent& cam)
    {
        // 1) Позиция
        const glm::vec3 cameraPos = pos.toGLMVec();

        // 2) Ориентация (вперёд = -Z, вверх = +Y)
        glm::quat q = glm::normalize(rot.toQuat());
        glm::vec3 forward = q * glm::vec3(0.f, 0.f, -1.f);
        glm::vec3 up      = q * glm::vec3(0.f, 1.f,  0.f);

        // На случай вырождения: обеспечим ортогональность up к forward
        if (glm::abs(glm::dot(forward, up)) > 0.999f) {
            // выберем другой временный up и ортонормируем
            glm::vec3 tmp = glm::abs(forward.y) < 0.9f ? glm::vec3(0,1,0) : glm::vec3(1,0,0);
            glm::vec3 right = glm::normalize(glm::cross(forward, tmp));
            up = glm::normalize(glm::cross(right, forward));
        }

        // 3) View
        const glm::mat4 view = glm::lookAt(cameraPos, cameraPos + forward, up);

        // 4) Aspect из реального render size
        const float aspect = (m_renderSize.second == 0)
                           ? (16.f/9.f)
                           : float(m_renderSize.first) / float(m_renderSize.second);

        // 5) Projection (OpenGL, NDC z ∈ [-1,1])
        const glm::mat4 proj = glm::perspective(glm::radians(cam.fov), aspect,
                                                cam.nearClip, cam.farClip);

        // 6) В буфер камеры
        m_cameraData.position   = glm::vec4(cameraPos, 1.0f);
        m_cameraData.view       = view;
        m_cameraData.projection = proj;
    });        }
   };
}
