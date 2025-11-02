#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>

#include "../../ResourceModule/ResourceManager.h"
#include "IFramebuffer.h"
#include "IMesh.h"
#include "IShader.h"
#include "ITexture.h"

namespace RenderModule
{
struct DirectionalLight
{
    glm::vec4 direction;
    glm::vec4 color;
    float intensity;
};

struct PointLight
{
    glm::vec4 position;
    glm::vec4 color;
    float intensity;
};

struct RenderObject
{
    ShaderUniformBlock_ModelData modelMatrix;
    std::shared_ptr<IMesh> mesh;
    std::shared_ptr<ITexture> texture;
    std::shared_ptr<IShader> shader;
};

enum RenderPassType
{
    SHADOW,
    REFLECTION,
    LIGHT,
    FINAL,
    CUSTOM,
    TEXTURE
};

struct RenderPassData
{
    RenderPassType renderPassType{CUSTOM};
    std::shared_ptr<IFramebuffer> framebuffer;
    std::unordered_map<std::shared_ptr<IShader>, std::unordered_map<std::shared_ptr<IMesh>, std::vector<RenderObject>>>
        batches;
    std::unordered_map<std::string, void *> textures;

    void render()
    {
    }

    void clear()
    {
        batches.clear();
    }
};

struct CameraData
{
    glm::mat4 projMatrix{1.0f};
    glm::mat4 viewMatrix{1.0f};
    glm::vec4 cameraPosition{1.0f};
};
struct RenderPipelineData
{
    glm::mat4 projMatrix{1.0f};
    glm::mat4 viewMatrix{1.0f};
    glm::vec4 cameraPosition{1.0f};

    DirectionalLight directionalLight;
    std::vector<PointLight> pointLights;

    RenderPassData shadowPass{.renderPassType = SHADOW};
    RenderPassData reflectionPass{.renderPassType = REFLECTION};
    RenderPassData lightPass{.renderPassType = LIGHT};
    RenderPassData finalPass{.renderPassType = FINAL};
    RenderPassData texturePass{.renderPassType = TEXTURE};
    RenderPassData customPass{.renderPassType = CUSTOM};

    void clear()
    {
        pointLights.clear();

        shadowPass.clear();
        reflectionPass.clear();
        lightPass.clear();
        finalPass.clear();
        texturePass.clear();
        customPass.clear();
    }
};
} // namespace RenderModule
